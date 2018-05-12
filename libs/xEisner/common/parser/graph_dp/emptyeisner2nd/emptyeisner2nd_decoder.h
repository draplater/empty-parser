// // Created by chenyufei on 17-8-22.
//

#ifndef XEISNER_EMPTYEISNER2ND_DECODER_H
#define XEISNER_EMPTYEISNER2ND_DECODER_H

#include "emptyeisner2nd_depparser.h"

namespace emptyeisner2nd {
    class Decoder : public DepParser {
    public:
        Decoder();

        std::vector<ECArc> decodePure1st(size_t length,
                                         size_t empty_size,
                                         tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]);

        std::vector<emptyeisner2nd::ECArc> decodePure(size_t length, size_t empty_size,
                                       tscore firstOrder[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                       tscore secondOrder1[MAX_EMPTY_SIZE + 1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                       tscore secondOrderSolid[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                       tscore secondOrderMid[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE],
                                       tscore secondOrderOut[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]);

        tscore baseArcScore(const int &p, const int &c) override;

        tscore biSiblingArcScore(const int &p, const int &c, const int &c2) override;

        tscore (*firstOrder)[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
        tscore (*secondOrder1)[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
        tscore (*secondOrderSolid)[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
        tscore (*secondOrderMid)[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
        tscore (*secondOrderOut)[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
        bool use2nd;
    };
}
#endif //XEISNER_EMPTYEISNER2ND_DECODER_H
