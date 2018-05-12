from operator import itemgetter
import os
import numpy as np
import dynet as dn

from logger import logger
from max_sub_tree.mstlstm_empty import SentenceWithEmpty, MaxSubTreeWithEmptyParser, emptyeisner2nd
from max_sub_tree.xeisner import MAX_SENT_SIZE_EMPTY


def joint_predict(sentences, model_empty, model_noempty):
    k = 0.5
    for iSentence, sentence in enumerate(sentences):
        if len(sentence) >= MAX_SENT_SIZE_EMPTY - 1:
            logger.info("sent too long...")
            heads = [0 for _ in range(len(sentence))]
            labels = [None for _ in range(len(sentence))]
            yield sentence.to_simple_sentence(heads, labels, set())
            continue

        lstm_output_empty = model_empty.network.get_lstm_output(sentence)
        scores_empty, _ = model_empty.network.get_complete_scores(lstm_output_empty)
        scores_ec, _ = model_empty.network_for_emptys.get_complete_scores(lstm_output_empty)

        lstm_output_noempty = model_noempty.network.get_lstm_output(sentence)
        scores_noempty, _ = model_noempty.network.get_complete_scores(lstm_output_noempty)

        scores_all = np.stack([k * scores_empty + (1 - k) * scores_noempty, scores_ec])
        exprs2nd2, scores2nd2, exprs2nd3, scores2nd3 = model_empty.network3.get_complete_scores(lstm_output_empty)
        _, scores2nd2_noempty, _, scores2nd3_noempty = model_noempty.network3.get_complete_scores(lstm_output_noempty)
        _, _, exprs_mid, scores_mid = model_empty.network3_for_emptys_mid.get_complete_scores(lstm_output_empty, False)
        exprs2nd2_ec, scores2nd2_ec, exprs_out, scores_out = model_empty.network3_for_emptys_out.get_complete_scores(
            lstm_output_empty)
        scores2nd2_all = k *  scores2nd2 + (1 - k) * scores2nd2_noempty
        scores2nd3_all = k *  scores2nd3 + (1 - k) * scores2nd3_noempty
        scores_all_ec = np.stack([scores2nd2_all, scores2nd2_ec])
        heads, emptys = emptyeisner2nd(scores_all, scores_all_ec, scores2nd3_all, scores_mid, scores_out)
        emptys_dict = {i: "*" for i in emptys}

        labels = [None for _ in range(len(sentence))]
        edges = [(head, "_", modifier) for modifier, head in enumerate(heads[1:], 1)]
        # for edge, scores_expr_empty, scores_expr_noempty in \
        for edge, scores_expr_empty in \
                zip(edges,
                    model_empty.network.get_label_scores(lstm_output_empty, edges),
                    # model_noempty.network.get_label_scores(lstm_output_noempty, edges)
                    ):
            head, _, modifier = edge
            # labels_scores = k * scores_expr_empty.npvalue() + (1 - k) * scores_expr_noempty.npvalue()
            labels_scores = scores_expr_empty.npvalue() 
            labels[modifier] = \
                model_empty.network.irels[max(enumerate(labels_scores), key=itemgetter(1))[0]]

        for empty_node, scores_expr in \
                zip(emptys,
                    model_empty.label_network_for_emptys.get_label_scores(lstm_output_empty, emptys),
                    ):
            labels_scores = scores_expr.npvalue()
            best_idx = max(enumerate(labels_scores), key=itemgetter(1))[0]
            emptys_dict[empty_node] = model_empty.statistics.emptys.int_to_word[best_idx]

        dn.renew_cg()

        result = sentence.to_simple_sentence(heads, labels, emptys_dict.items())
        yield result


def run(test_file, output_file, model_empty_file, model_noempty_file):
    sentences = SentenceWithEmpty.from_file(test_file)
    model_empty = MaxSubTreeWithEmptyParser.load(model_empty_file)
    model_noempty = MaxSubTreeWithEmptyParser.load(model_noempty_file)
    with open(output_file, "w") as f:
        for output_sent in joint_predict(sentences, model_empty, model_noempty):
            f.write(output_sent.to_string())
    SentenceWithEmpty.evaluate_with_external_program(test_file, output_file)


def main():
    home = os.path.expanduser("~")
    run(
        home + "/Development/large-data/emptynode-data/cn.dev.auto.short.ec",
        home + "/Public/empty-2/dev-2nd-auto-joint",
        home + "/Development/nullnode/20180203/model-cn_label_empty_True_label/model.9",
        home + "/Development/nullnode/20180203/model-cn_label_noempty_True+label/model.4",
    )


if __name__ == '__main__':
    main()
