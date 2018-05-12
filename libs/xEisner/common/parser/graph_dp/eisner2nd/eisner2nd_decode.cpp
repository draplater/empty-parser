//
// Created by chenyufei on 17-8-20.
//

#include <include/matrix_view.h>
#include <stack>
#include "eisner2nd_state.h"
#define MAX_SENTENCE_SIZE_2ND 256

namespace eisner2nd {
    static tscore padding[100000];
    static MatrixView::Array2DRotated<tscore> paddingMat2d(const_cast<tscore *>(padding), 1, 1);
    static MatrixView::Array3DRotated<tscore> paddingMat3d(const_cast<tscore *>(padding), 1, 1, 1);

    std::vector<Arc> decode(int sentLength,
                            MatrixView::Array2DRotated<tscore> firstOrder,
                            MatrixView::Array2DRotated<tscore> secondOrder1,
                            MatrixView::Array3DRotated<tscore> secondOrder2) {
        StateItem m_lItems[MAX_SENTENCE_SIZE_2ND][MAX_SENTENCE_SIZE_2ND];
        for (int d = 2; d <= sentLength + 1; ++d) {
            for (int i = 0, max_i = sentLength - d + 1; i < max_i; ++i) {

                int l = i + d - 1;
                StateItem &item = m_lItems[d][i];
                const tscore &l2r_arc_score = firstOrder(i, l);
                const tscore &r2l_arc_score = firstOrder(l, i);

                // initialize
                item.init(i, l);

                // jux
                for (int s = i; s < l; ++s) {
                    item.updateJUX(s, m_lItems[s - i + 1][i].l2r.getScore() + m_lItems[l - s][s + 1].r2l.getScore());
                }

                for (int k = i + 1; k < l; ++k) {

                    StateItem &litem = m_lItems[k - i + 1][i];
                    StateItem &ritem = m_lItems[l - k + 1][k];

                    // solid both
                    item.updateL2RSolidBoth(k, ritem.jux.getScore() + litem.l2r_solid_both.getScore() + l2r_arc_score +
                                               secondOrder2(i, k, l));
                    item.updateR2LSolidBoth(k, litem.jux.getScore() + ritem.r2l_solid_both.getScore() + r2l_arc_score +
                                               secondOrder2(l, k, i));

                    // complete
                    item.updateL2R(k, litem.l2r_solid_both.getScore() + ritem.l2r.getScore());
                    item.updateR2L(k, ritem.r2l_solid_both.getScore() + litem.r2l.getScore());
                }
                // solid both
                item.updateL2RSolidBoth(i,
                                        m_lItems[d - 1][i + 1].r2l.getScore() + l2r_arc_score + secondOrder1(i, l));
                item.updateR2LSolidBoth(l, m_lItems[d - 1][i].l2r.getScore() + r2l_arc_score + secondOrder1(l, i));
                // complete
                item.updateL2R(l, item.l2r_solid_both.getScore());
                item.updateR2L(i, item.r2l_solid_both.getScore());
            }
            // root
            StateItem &item = m_lItems[d][sentLength - d + 1];
            item.init(sentLength - d + 1, sentLength);
            // solid both
            item.updateR2LSolidBoth(sentLength, m_lItems[d - 1][item.left].l2r.getScore() +
                                                firstOrder(item.right, item.left) +
                                                secondOrder1(item.left + d - 1, item.left));
            // complete
            item.updateR2L(item.left, item.r2l_solid_both.getScore());
            for (int i = item.left, s = item.left + 1, j = sentLength + 1; s < j - 1; ++s) {
                item.updateR2L(s, m_lItems[j - s][s].r2l_solid_both.getScore() + m_lItems[s - i + 1][i].r2l.getScore());
            }
        }


        std::vector<Arc> resultArcs;
        std::stack<std::tuple<int, int>> stack;
        m_lItems[sentLength + 1][0].type = R2L;
        stack.push(std::tuple<int, int>(sentLength + 1, 0));

        while (!stack.empty()) {

            std::tuple<int, int> span = std::move(stack.top());
            stack.pop();

            int s = -1;
            StateItem &item = m_lItems[std::get<0>(span)][std::get<1>(span)];

            if (item.left == item.right) {
                continue;
            }
            switch (item.type) {
                case JUX:
                    if (item.left < item.right - 1) {
                        s = item.jux.getSplit();
                        m_lItems[s - item.left + 1][item.left].type = L2R;
                        stack.push(std::tuple<int, int>(s - item.left + 1, item.left));
                        m_lItems[item.right - s][s + 1].type = R2L;
                        stack.push(std::tuple<int, int>(item.right - s, s + 1));
                    }
                    break;
                case L2R:
                    s = item.l2r.getSplit();

                    if (item.left < item.right - 1) {
                        m_lItems[s - item.left + 1][item.left].type = L2R_SOLID_BOTH;
                        stack.push(std::tuple<int, int>(s - item.left + 1, item.left));
                        m_lItems[item.right - s + 1][s].type = L2R;
                        stack.push(std::tuple<int, int>(item.right - s + 1, s));
                    } else {
                        resultArcs.emplace_back(item.left, item.right);
                    }
                    break;
                case R2L:
                    s = item.r2l.getSplit();

                    if (item.left < item.right - 1) {
                        m_lItems[item.right - s + 1][s].type = R2L_SOLID_BOTH;
                        stack.push(std::tuple<int, int>(item.right - s + 1, s));
                        m_lItems[s - item.left + 1][item.left].type = R2L;
                        stack.push(std::tuple<int, int>(s - item.left + 1, item.left));
                    } else {
                        resultArcs.emplace_back(item.right == sentLength ? -1 : item.right, item.left);
                    }
                    break;
                case L2R_SOLID_BOTH:
                    resultArcs.emplace_back(item.left, item.right);

                    s = item.l2r_solid_both.getSplit();

                    if (s == item.left) {
                        m_lItems[item.right - s][s + 1].type = R2L;
                        stack.push(std::tuple<int, int>(item.right - s, s + 1));
                    } else {
                        m_lItems[s - item.left + 1][item.left].type = L2R_SOLID_BOTH;
                        stack.push(std::tuple<int, int>(s - item.left + 1, item.left));
                        m_lItems[item.right - s + 1][s].type = JUX;
                        stack.push(std::tuple<int, int>(item.right - s + 1, s));
                    }
                    break;
                case R2L_SOLID_BOTH:
                    resultArcs.emplace_back(item.right == sentLength ? -1 : item.right, item.left);

                    s = item.r2l_solid_both.getSplit();

                    if (s == item.right) {
                        m_lItems[s - item.left][item.left].type = L2R;
                        stack.push(std::tuple<int, int>(s - item.left, item.left));
                    } else {
                        m_lItems[item.right - s + 1][s].type = R2L_SOLID_BOTH;
                        stack.push(std::tuple<int, int>(item.right - s + 1, s));
                        m_lItems[s - item.left + 1][item.left].type = JUX;
                        stack.push(std::tuple<int, int>(s - item.left + 1, item.left));
                    }
                    break;
                default:
                    break;
            }
        }
        return resultArcs;
    }
}

extern "C" {
size_t eisner2nd_decode(size_t length,
                        int result[][2],
                        tscore *firstOrder) {
    using namespace eisner2nd;
    auto edges = decode(static_cast<int>(length),
                        MatrixView::Array2DRotated<tscore>(firstOrder, length + 1, length + 1),
                        paddingMat2d,
                        paddingMat3d);
    for (int i = 0; i < edges.size(); i++) {
        result[i][0] = edges[i].first();
        result[i][1] = edges[i].second();
    }
    return edges.size();
}

size_t eisner2nd_decode2nd(size_t length, int result[][2],
                           tscore *firstOrder,
                           tscore *secondOrder1,
                           tscore *secondOrder2) {
    using namespace eisner2nd;
    auto edges = decode(static_cast<int>(length),
                        MatrixView::Array2DRotated<tscore>(firstOrder, length + 1, length + 1),
                        MatrixView::Array2DRotated<tscore>(secondOrder1, length + 1,
                                                           length + 1),
                        MatrixView::Array3DRotated<tscore>(
                                secondOrder2, length + 1, length + 1, length + 1));
    for (int i = 0; i < edges.size(); i++) {
        result[i][0] = edges[i].first();
        result[i][1] = edges[i].second();
    }
    return edges.size();
}
}


int main() {
    using namespace eisner2nd;
    using namespace std;
    auto ret = decode(10, paddingMat2d, paddingMat2d, paddingMat3d);
    printf("\n");
}
