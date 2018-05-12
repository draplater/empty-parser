from operator import itemgetter
import os
import numpy as np
import dynet as dn

from logger import logger
from max_sub_tree.mstlstm_empty import SentenceWithEmpty, MaxSubTreeWithEmptyParser, emptyeisner2nd
from max_sub_tree.xeisner import MAX_SENT_SIZE_EMPTY


def joint_predict(sentences, model_1st, model_2nd):
    k = 0.5
    for iSentence, sentence in enumerate(sentences):
        if len(sentence) >= MAX_SENT_SIZE_EMPTY - 1:
            logger.info("sent too long...")
            heads = [0 for _ in range(len(sentence))]
            labels = [None for _ in range(len(sentence))]
            yield sentence.to_simple_sentence(heads, labels, set())
            continue

        lstm_output_1st = model_1st.network.get_lstm_output(sentence)
        scores_1st, _ = model_1st.network.get_complete_scores(lstm_output_1st)
        scores_ec_1st, _ = model_1st.network_for_emptys.get_complete_scores(lstm_output_1st)

        lstm_output_2nd = model_2nd.network.get_lstm_output(sentence)
        scores_2nd, _ = model_2nd.network.get_complete_scores(lstm_output_2nd)
        scores_ec_2nd, _ = model_2nd.network_for_emptys.get_complete_scores(lstm_output_2nd)

        scores_all = np.stack([k * scores_1st + (1 - k) * scores_2nd, k * scores_ec_1st + (1 - k) * scores_ec_2nd])
        exprs2nd2, scores2nd2, exprs2nd3, scores2nd3 = model_2nd.network3.get_complete_scores(lstm_output_2nd)
        _, _, exprs_mid, scores_mid = model_2nd.network3_for_emptys_mid.get_complete_scores(lstm_output_2nd, False)
        exprs2nd2_ec, scores2nd2_ec, exprs_out, scores_out = model_2nd.network3_for_emptys_out.get_complete_scores(
            lstm_output_2nd)
        scores_all_ec = np.stack([scores2nd2, scores2nd2_ec])
        heads, emptys = emptyeisner2nd(scores_all, scores_all_ec, scores2nd3, scores_mid, scores_out)
        emptys_dict = {i: "*" for i in emptys}

        labels = [None for _ in range(len(sentence))]
        edges = [(head, "_", modifier) for modifier, head in enumerate(heads[1:], 1)]
        for edge, scores_expr_1st, scores_expr_2nd in \
                zip(edges,
                    model_1st.network.get_label_scores(lstm_output_1st, edges),
                    model_2nd.network.get_label_scores(lstm_output_2nd, edges)
                    ):
            head, _, modifier = edge
            labels_scores = k * scores_expr_1st.npvalue() + (1 - k) * scores_expr_2nd.npvalue()
            labels[modifier] = \
                model_1st.network.irels[max(enumerate(labels_scores), key=itemgetter(1))[0]]

        for empty_node, scores_expr_1st, scores_expr_2nd in \
                zip(emptys,
                    model_1st.label_network_for_emptys.get_label_scores(lstm_output_1st, emptys),
                    model_2nd.label_network_for_emptys.get_label_scores(lstm_output_2nd, emptys),
                    ):
            labels_scores = k * scores_expr_1st.npvalue() + (1 - k) * scores_expr_2nd.npvalue()
            best_idx = max(enumerate(labels_scores), key=itemgetter(1))[0]
            emptys_dict[empty_node] = model_1st.statistics.emptys.int_to_word[best_idx]

        dn.renew_cg()

        result = sentence.to_simple_sentence(heads, labels, emptys_dict.items())
        yield result


def run(test_file, output_file, model_1st_file, model_2nd_file):
    sentences = SentenceWithEmpty.from_file(test_file)
    model_1st = MaxSubTreeWithEmptyParser.load(model_1st_file)
    model_2nd = MaxSubTreeWithEmptyParser.load(model_2nd_file)
    with open(output_file, "w") as f:
        for output_sent in joint_predict(sentences, model_1st, model_2nd):
            f.write(output_sent.to_string())
    SentenceWithEmpty.evaluate_with_external_program(test_file, output_file)


def main():
    home = os.path.expanduser("~")
    run(
        home + "/Development/large-data/emptynode-data/cn.tst.auto.ec",
        "/tmp/abcdefg-test-a",
        home + "/Development/nullnode/20180203/model-cn_label_empty_False-2layers/model.14",
        home + "/Development/nullnode/20180203/model-cn_label_empty_True_label/model.9",
    )


if __name__ == '__main__':
    main()
