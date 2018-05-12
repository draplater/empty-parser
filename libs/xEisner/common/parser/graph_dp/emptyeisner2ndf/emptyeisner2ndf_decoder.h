// // Created by chenyufei on 17-8-22.
//

#ifndef XEISNER_EMPTYEISNER2NDF_DECODER_H
#define XEISNER_EMPTYEISNER2NDF_DECODER_H

#include "emptyeisner2ndf_depparser.h"

namespace emptyeisner2ndf {
	class Decoder : public DepParser {
	public:
		Decoder();
		        std::vector<ECArc> decodePure1st(size_t length,
												 size_t empty_size,
                                    tscore firstOrder[MAX_EMPTY_SIZE+1][MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE]);

		void initArcScore(const int &d) override;

		void initBiSiblingArcScore(const int &d) override;

		tscore (*firstOrder)[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
	};
}
#endif //XEISNER_EMPTYEISNER2NDF_DECODER_H
