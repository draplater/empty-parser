//
// Created by chenyufei on 17-8-22.
//

#include "emptyeisner2nd_decoder.h"

emptyeisner2nd::Decoder::Decoder() : DepParser("", "", ParserState::PARSE) {}

std::vector<emptyeisner2nd::ECArc>
emptyeisner2nd::Decoder::decodePure1st(size_t length, size_t empty_size,
                                       tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]) {
    this->m_nSentenceLength = static_cast<int>(length);
    for (int i = 0; i < empty_size; i++) {
        this->m_tEmptys.add(std::to_string(i));
    }
    this->firstOrder = firstOrder;
    this->use2nd = false;
    decode();
    decodeArcs();
    return m_vecTrainECArcs;
}

std::vector<emptyeisner2nd::ECArc>
emptyeisner2nd::Decoder::decodePure(size_t length, size_t empty_size,
                                    tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                    tscore secondOrder1[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                    tscore secondOrderSolid[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                    tscore secondOrderMid[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                    tscore secondOrderOut[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]) {
    this->m_nSentenceLength = static_cast<int>(length);
    for (int i = 0; i < empty_size; i++) {
        this->m_tEmptys.add(std::to_string(i));
    }
    this->use2nd = true;
    this->firstOrder = firstOrder;
    this->secondOrder1 = secondOrder1;
    this->secondOrderSolid = secondOrderSolid;
    this->secondOrderMid = secondOrderMid;
    this->secondOrderOut = secondOrderOut;
    decode();
    decodeArcs();
    return m_vecTrainECArcs;
}

tscore emptyeisner2nd::Decoder::baseArcScore(const int &p, const int &c) {
    int pos_c = DECODE_EMPTY_POS(c);
    int tag_c = DECODE_EMPTY_TAG(c);
    return firstOrder[tag_c][p][pos_c];
}

tscore emptyeisner2nd::Decoder::biSiblingArcScore(const int &p, const int &c, const int &c2) {
    if (!this->use2nd) {
        return 0;
    }

    int pos_c2 = DECODE_EMPTY_POS(c2);
    int tag_c2 = DECODE_EMPTY_TAG(c2);

    if (c == -1) {
        return this->secondOrder1[tag_c2][p][pos_c2];
    }

    int pos_c = DECODE_EMPTY_POS(c);
    int tag_c = DECODE_EMPTY_TAG(c);

    if (tag_c != 0 && tag_c2 != 0) {
        throw std::runtime_error("mid + out is not allowed");
    }

    if (tag_c != 0) {
        return this->secondOrderMid[p][pos_c][pos_c2];
    }

    if (tag_c2 != 0) {
        return this->secondOrderOut[p][pos_c][pos_c2];
    }

    return this->secondOrderSolid[p][pos_c][pos_c2];
}

extern "C" {
int emptyeisner1st_decode(size_t length, size_t empty_size,
                          int result[][2],
                          tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]) {
    using namespace emptyeisner2nd;
    Decoder instance;
    const std::vector<ECArc> &ret = instance.decodePure1st(length, empty_size, firstOrder);
    for (int i = 0; i < ret.size(); i++) {
        result[i][0] = ret[i].first();
        result[i][1] = ret[i].second();
    }
    return static_cast<int>(ret.size());
}
}

extern "C" {
int emptyeisner2nd_decode(size_t length, size_t empty_size,
                          int result[][2],
                          tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                          tscore secondOrder1[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                          tscore secondOrderSolid[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                          tscore secondOrderMid[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                          tscore secondOrderOut[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]) {
    using namespace emptyeisner2nd;
    Decoder instance;
    const std::vector<ECArc> &ret = instance.decodePure(length, empty_size, firstOrder, secondOrder1,
                                                        secondOrderSolid, secondOrderMid, secondOrderOut);
    for (int i = 0; i < ret.size(); i++) {
        result[i][0] = ret[i].first();
        result[i][1] = ret[i].second();
    }
    return static_cast<int>(ret.size());
}
}
