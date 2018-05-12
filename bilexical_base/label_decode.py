from operator import itemgetter
import dynet as dn
import numpy as np

import graph_utils
from vocab_utils import Dictionary


def hinge(edges, labels_exprs,
          label_dict,  # type: Dictionary
          is_train):
    labeled_edges = []
    loss = dn.scalarInput(0.0)
    for edge, r_scores_expr in zip(edges, labels_exprs):
        head, label, modifier = edge
        if head == 0:
            if not is_train:
                labeled_edges.append(graph_utils.Edge(edge.source, "ROOT", edge.target))
            continue
        r_scores = r_scores_expr.value()
        if is_train:
            gold_label_index = label_dict.word_to_int[label]
            r_scores[gold_label_index] -= 1

        label_index = max(((l, scr) for l, scr in enumerate(r_scores)), key=itemgetter(1))[0]

        if is_train:
            if label_index != gold_label_index:
                loss += r_scores_expr[label_index] - r_scores_expr[gold_label_index] + 1
        else:
            label = label_dict.int_to_word[label_index]
            labeled_edges.append(graph_utils.Edge(edge.source, label, edge.target))

    if is_train:
        return loss
    else:
        return labeled_edges


def softmax(edges, labels_exprs,
            label_dict,  # type: Dictionary
            is_train):
    labeled_edges = []
    loss = dn.scalarInput(0.0)
    for edge, r_scores_expr in zip(edges, labels_exprs):
        head, label, modifier = edge
        if head == 0:
            if not is_train:
                labeled_edges.append(graph_utils.Edge(edge.source, "ROOT", edge.target))
            continue
        if is_train:
            gold_label_index = label_dict.word_to_int[label]
            loss += dn.pickneglogsoftmax(r_scores_expr, gold_label_index)
        else:
            label_index = np.argmax(r_scores_expr.value())
            label = label_dict.int_to_word[label_index]
            labeled_edges.append(graph_utils.Edge(edge.source, label, edge.target))
    if is_train:
        return loss
    else:
        return labeled_edges


label_decoders = {"greedy": hinge, "softmax": softmax}
