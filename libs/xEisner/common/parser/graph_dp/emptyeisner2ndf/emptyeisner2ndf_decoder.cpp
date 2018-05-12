//
// Created by chenyufei on 17-8-22.
//

#include "emptyeisner2ndf_decoder.h"

emptyeisner2ndf::Decoder::Decoder() : DepParser("", "", ParserState::PARSE) {}

std::vector<emptyeisner2ndf::ECArc>
emptyeisner2ndf::Decoder::decodePure1st(size_t length, size_t empty_size,
                                       tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]) {
    this->m_nSentenceLength = static_cast<int>(length);
    for(int i=0; i<empty_size; i++) {
        this->m_tEmptys.add(std::to_string(i));
    }
    this->firstOrder = firstOrder;
    decode();
    decodeArcs();
    return m_vecTrainECArcs;
}

extern "C" {
int emptyeisner1stf_decode(size_t length, size_t empty_size,
                          int result[][2],
                          tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]) {
    using namespace emptyeisner2ndf;
    Decoder instance;
    const std::vector<ECArc> &ret = instance.decodePure1st(length, empty_size, firstOrder);
    for(int i=0; i<ret.size();  i++) {
        result[i][0] = ret[i].first();
        result[i][1] = ret[i].second();
    }
    return static_cast<int>(ret.size());
}
}

void emptyeisner2ndf::Decoder::initArcScore(const int &d) {
    for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
        int l = i, r = i + d - 1;
        if (d > 1) {
            m_lArcScore[l][r][0][0] = firstOrder[0][l][r];
            m_lArcScore[l][r][1][0] = firstOrder[0][r][l];
        }
        for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
            m_lArcScore[l][r][0][ec] = firstOrder[ec][l][r+1];
            m_lArcScore[l][r][1][ec] = firstOrder[ec][r][l];
        }
    }
    if (d > 1) {
        int l = m_nSentenceLength - d + 1, r = m_nSentenceLength;
        m_lArcScore[l][r][1][0] = firstOrder[0][r][l];
    }
}

void emptyeisner2ndf::Decoder::initBiSiblingArcScore(const int &d) {
    for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
        int l = i + d - 1;
        if (d > 1) {
            m_lBiSiblingScore[i][0][i][0][0] = 0;
            m_lBiSiblingScore[i][1][i][0][0] = 0;
        }
        if (d == 1) {
            for (int out_ec = 1; out_ec <= MAX_EMPTY_SIZE; ++out_ec) {
                m_lBiSiblingScore[i][0][i][0][out_ec] = 0;
                m_lBiSiblingScore[i][1][i][0][out_ec] = 0;
            }
        } else {
            for (int out_ec = 1; out_ec <= MAX_EMPTY_SIZE; ++out_ec) {
                m_lBiSiblingScore[i][0][i][0][out_ec] = 0;
                m_lBiSiblingScore[i][1][i][0][out_ec] = 0;
            }
        }
        for (int mid_ec = 1; mid_ec <= MAX_EMPTY_SIZE; ++mid_ec) {
            m_lBiSiblingScore[i][0][i][mid_ec][0] = 0;
            m_lBiSiblingScore[i][1][i][mid_ec][0] = 0;
            //if (i == 30 && l == 31) std::cout << "l2r inside score = " << m_lBiSiblingScore[i][0][i][mid_ec][0] << std::endl;
        }
        for (int k = i + 1, l = i + d - 1; k < l; ++k) {
            if (d > 1) {
                m_lBiSiblingScore[i][0][k][0][0] = 0;
                m_lBiSiblingScore[i][1][k][0][0] = 0;
            }
            for (int mid_ec = 1; mid_ec <= MAX_EMPTY_SIZE; ++mid_ec) {
                m_lBiSiblingScore[i][0][k][mid_ec][0] = 0;
                m_lBiSiblingScore[i][1][k][mid_ec][0] = 0;
            }
            for (int out_ec = 1; out_ec <= MAX_EMPTY_SIZE; ++out_ec) {
                m_lBiSiblingScore[i][0][k][0][out_ec] = 0;
                m_lBiSiblingScore[i][1][k][0][out_ec] = 0;
            }
        }
    }
    m_lBiSiblingScore[m_nSentenceLength - d + 1][1][m_nSentenceLength - d + 1][0][0] = 0;
}
