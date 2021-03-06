import base64
import pickle
import random

import dynet as dn
import time
from operator import itemgetter

import numpy as np
from six.moves import range
from multiprocessing import Pool

import nn
from bilexical_base.label_decode import label_decoders
from common_utils import split_to_batches
from conll_reader import CoNLLUSentence, CoNLLUNode
from edge_eval_network import EdgeEvaluationNetwork
from graph_utils import Edge
from vocab_utils import Statistics
from logger import logger
from max_sub_tree.decoder import decoders
from nn import activations
from parser_base import TreeParserBase


class PrintLogger(object):
    def __init__(self):
        self.reset()

    def reset(self):
        self.total_loss_value = 0.0
        self.total_edge = 0
        self.correct_edge = 0
        self.start = time.time()

    def print(self, sentence_idx):
        logger.info(
            'Processing sentence number: %d, Loss: %.2f, '
            'Correctness: %.2f, Time: %.2f',
            sentence_idx, self.total_loss_value,
            self.correct_edge / self.total_edge * 100,
            time.time() - self.start
        )
        self.reset()


class MaxSubTreeParser(TreeParserBase):
    # smaller batch size is better for this task
    default_batch_size = 2
    default_test_batch_size = 64

    @classmethod
    def add_parser_arguments(cls, arg_parser):
        super(MaxSubTreeParser, cls).add_parser_arguments(arg_parser)
        group = arg_parser.add_argument_group(cls.__name__)
        group.add_argument("--optimizer", type=str, dest="optimizer", default="adam", choices=nn.trainers.keys())
        group.add_argument("--decoder", type=str, dest="decoder", default="eisner", choices=decoders)
        group.add_argument("--label-decoder", type=str, dest="label_decoder", default="greedy",
                           choices=["greedy", "softmax"])
        group.add_argument("--cost-augment", action="store_true", dest="cost_augment", default=True)
        group.add_argument("--batch-size", type=int, dest="batch_size", default=cls.default_batch_size)
        group.add_argument("--model-format", dest="model_format", choices=nn.model_formats, default="pickle")
        EdgeEvaluationNetwork.add_parser_arguments(arg_parser)

    @classmethod
    def add_predict_arguments(cls, arg_parser):
        super(MaxSubTreeParser, cls).add_predict_arguments(arg_parser)
        group = arg_parser.add_argument_group(cls.__name__)
        group.add_argument("--model-format", dest="model_format", choices=nn.model_formats, default=None)

    @classmethod
    def add_common_arguments(cls, arg_parser):
        super(MaxSubTreeParser, cls).add_common_arguments(arg_parser)
        group = arg_parser.add_argument_group(cls.__name__ + " (common)")
        group.add_argument("--test-batch-size", type=int, dest="test_batch_size", default=cls.default_test_batch_size)
        group.add_argument("--concurrent-count", type=int, dest="concurrent_count", default=5)

    def __init__(self, options, train_sentences=None):
        self.model = dn.Model()
        self.statistics = Statistics.from_sentences(train_sentences)
        self.container = nn.Container(self.model)
        self.network = EdgeEvaluationNetwork(self.container, self.statistics, options)
        self.optimizer = nn.get_optimizer(self.model, options)
        self.decoder = decoders[options.decoder]
        self.label_decoder = label_decoders[options.label_decoder]
        self.labelsFlag = options.labelsFlag
        self.options = options
        if "func" in options:
            del options.func

    def __getstate__(self):
        ret = dict(self.__dict__)
        for i in ("model", "optimizer", "container", "network"):
            del ret[i]
        return ret

    def save(self, prefix):
        nn.model_save_helper("pickle", prefix, self.container, self)

    @classmethod
    def load(cls,
             prefix,  # type: str
             new_options=None):
        """
        :param prefix: model file name prefix
        :rtype: MaxSubGraphParser
        """
        model = dn.Model()
        parser, savable = nn.model_load_helper(None, prefix, model)
        parser.options.__dict__.update(new_options.__dict__)
        parser.model = model
        parser.container = savable
        parser.network = parser.container.components[0]
        parser.optimizer = nn.get_optimizer(model, parser.options)
        return parser

    def predict_session(self, sentence, pool):
        lstm_output = self.network.get_lstm_output(sentence)
        length = len(sentence)
        raw_exprs = self.network.edge_eval.get_complete_raw_exprs(lstm_output)
        yield raw_exprs

        scores = self.network.edge_eval.raw_exprs_to_scores(raw_exprs, length)

        heads_future = pool.apply_async(self.decoder, (scores,))
        yield None
        heads = heads_future.get()

        labels = [None for _ in range(len(sentence))]
        if self.labelsFlag:
            edges = [Edge(head, "_", modifier) for modifier, head in enumerate(heads[1:], 1)]
            labels_exprs = list(self.network.label_eval.get_label_scores(lstm_output, edges))
            yield labels_exprs
            edges = self.label_decoder(edges, labels_exprs, self.statistics.labels, False)
            for edge in edges:
                labels[edge.target] = edge.label
        else:
            yield []

        def convert_node(node):
            # noinspection PyArgumentList
            return CoNLLUNode(node.id, node.form, node.lemma, node.cpos,
                              node.pos, node.feats,
                              heads[node.id], labels[node.id],
                              "_", "_")

        result = CoNLLUSentence(convert_node(node) for node in sentence if node.id > 0)
        if self.options.output_scores:
            # extract full edge scores
            edges = [(head, "_", modifier) for head in range(len(sentence))
                     for modifier in range(len(sentence))]
            edges_scores_all = np.array(list(i.value() for i in self.network.get_label_scores(lstm_output, edges)))
            edges_scores_all = edges_scores_all.reshape((len(sentence), len(sentence), len(self.network.rels)))
            result.comment = [base64.b64encode(pickle.dumps(scores)).decode()]
            result.comment.append(base64.b64encode(pickle.dumps(edges_scores_all)).decode()
                                  if self.labelsFlag else "No labels")
        yield result

    def predict(self, sentences):
        self.network.sent_embedding.rnn.disable_dropout()
        pool = Pool(self.options.concurrent_count)
        for sentence_idx, batch_idx, batch_sentences in split_to_batches(
                sentences, self.options.test_batch_size):
            sessions = [self.predict_session(sentence, pool)
                        for sentence in batch_sentences]
            all_exprs = [next(i) for i in sessions]
            if all_exprs:
                dn.forward(all_exprs)
            # spawn decoders
            for i in sessions:
                next(i)
            all_labels_exprs = [j for i in sessions for j in next(i)]
            if all_labels_exprs:
                dn.forward(all_labels_exprs)
            for i in sessions:
                yield next(i)
            dn.renew_cg()

    def training_session(self, sentence, print_logger, pool):
        lstm_output = self.network.get_lstm_output(sentence)
        length = len(sentence)
        raw_exprs = self.network.edge_eval.get_complete_raw_exprs(lstm_output)
        yield raw_exprs

        scores = self.network.edge_eval.raw_exprs_to_scores(raw_exprs, length)
        exprs = self.network.edge_eval.raw_exprs_to_exprs(raw_exprs, length)

        gold = [entry.parent_id for entry in sentence]
        heads_future = pool.apply_async(self.decoder,
                                        (scores, gold if self.options.cost_augment else None))
        yield None
        heads = heads_future.get()

        if self.labelsFlag:
            edges = [(head, sentence[modifier].relation, modifier)
                     for modifier, head in enumerate(gold[1:], 1)]
            label_exprs = list(self.network.get_label_scores(lstm_output, edges))
            yield label_exprs
            label_loss = self.label_decoder(edges, label_exprs, self.statistics.labels, True)
        else:
            label_loss = dn.scalarInput(0.0)
            yield []

        head_exprs = [(exprs[h][i] - exprs[g][i] + 1)
                      for i, (h, g) in enumerate(zip(heads, gold)) if
                      h != g]
        print_logger.correct_edge += len(sentence) - len(head_exprs)
        print_logger.total_edge += len(sentence)
        head_loss = dn.esum(head_exprs) if head_exprs else dn.scalarInput(0.0)
        yield label_loss + head_loss

    def train_gen(self, sentences, update=True):
        print_logger = PrintLogger()
        pool = Pool(self.options.concurrent_count)
        self.network.sent_embedding.rnn.set_dropout(self.options.lstm_dropout)
        print_per = (100 // self.options.batch_size + 1) * self.options.batch_size

        for sentence_idx, batch_idx, batch_sentences in split_to_batches(
                sentences, self.options.batch_size):
            if sentence_idx % print_per == 0 and sentence_idx != 0:
                print_logger.print(sentence_idx)
            sessions = [self.training_session(sentence, print_logger, pool)
                        for sentence in batch_sentences]
            all_exprs = [next(i) for i in sessions]
            if all_exprs:
                dn.forward(all_exprs)
            # spawn decoders
            for i in sessions:
                next(i)
            all_labels_exprs = [j for i in sessions for j in next(i)]
            if all_labels_exprs:
                dn.forward(all_labels_exprs)
            loss = sum(next(i) for i in sessions) / len(sessions)
            print_logger.total_loss_value += loss.value()
            if update:
                loss.backward()
                self.optimizer.update()
                dn.renew_cg()
            yield (loss if not update else None)

    def train(self, sentences):
        for _ in self.train_gen(sentences):
            pass
