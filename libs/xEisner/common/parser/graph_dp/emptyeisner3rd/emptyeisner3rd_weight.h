#ifndef _EMPTY_EISNER_WEIGHT_H
#define _EMPTY_EISNER_WEIGHT_H

#include "common/token/token.h"
#include "emptyeisner3rd_macros.h"
#include "common/parser/weight_base.h"
#include "include/learning/perceptron/packed_score.h"

namespace emptyeisner3rd {
	class Weight : public WeightBase {
	public:
		Token * m_pWords;
		Token * m_pPOSTags;
		Token * m_pEmptys;

		WordIntMap m_mapPw;
		POSTagIntMap m_mapPp;
		WordPOSTagIntMap m_mapPwp;

		WordIntMap m_mapCw;
		POSTagIntMap m_mapCp;
		WordPOSTagIntMap m_mapCwp;
		WordWordPOSTagPOSTagIntMap m_mapPwpCwp;
		WordPOSTagPOSTagIntMap m_mapPpCwp;
		WordPOSTagPOSTagIntMap m_mapPwpCp;
		WordWordPOSTagIntMap m_mapPwCwp;
		WordWordPOSTagIntMap m_mapPwpCw;
		TwoWordsIntMap m_mapPwCw;
		POSTagSet2IntMap m_mapPpCp;
		POSTagSet3IntMap m_mapPpBpCp;
		POSTagSet4IntMap m_mapPpPp1Cp_1Cp;
		POSTagSet4IntMap m_mapPp_1PpCp_1Cp;
		POSTagSet4IntMap m_mapPpPp1CpCp1;
		POSTagSet4IntMap m_mapPp_1PpCpCp1;

		POSTagSet2IntMap m_mapC1pC2p;
		POSTagSet3IntMap m_mapPpC1pC2p;
		TwoWordsIntMap m_mapC1wC2w;
		WordPOSTagIntMap m_mapC1wC2p;
		WordPOSTagIntMap m_mapC2wC1p;

		WordPOSTagPOSTagPOSTagIntMap m_mapPwC1pC2pC3p;
		WordPOSTagPOSTagPOSTagIntMap m_mapC1wPpC2pC3p;
		WordPOSTagPOSTagPOSTagIntMap m_mapC2wPpC1pC3p;
		WordPOSTagPOSTagPOSTagIntMap m_mapC3wPpC1pC2p;
		WordWordPOSTagPOSTagIntMap m_mapPwC1wC2pC3p;
		WordWordPOSTagPOSTagIntMap m_mapPwC2wC1pC3p;
		WordWordPOSTagPOSTagIntMap m_mapPwC3wC1pC2p;
		WordWordPOSTagPOSTagIntMap m_mapC1wC2wPpC3p;
		WordWordPOSTagPOSTagIntMap m_mapC1wC3wPpC2p;
		WordWordPOSTagPOSTagIntMap m_mapC2wC3wPpC1p;
		POSTagSet4IntMap m_mapPpC1pC2pC3p;
		WordPOSTagPOSTagIntMap m_mapC1wC2pC3p;
		WordPOSTagPOSTagIntMap m_mapC2wC1pC3p;
		WordPOSTagPOSTagIntMap m_mapC3wC1pC2p;
		WordWordPOSTagIntMap m_mapC1wC2wC3p;
		WordWordPOSTagIntMap m_mapC1wC3wC2p;
		WordWordPOSTagIntMap m_mapC2wC3wC1p;
		POSTagSet3IntMap m_mapC1pC2pC3p;
		TwoWordsIntMap m_mapC1wC3w;
		WordPOSTagIntMap m_mapC1wC3p;
		WordPOSTagIntMap m_mapC3wC1p;
		POSTagSet2IntMap m_mapC1pC3p;

	public:
		Weight(const std::string & sRead, const std::string & sRecord, Token * words, Token * postags, Token * emptys);
		~Weight();

		void loadScores();
		void saveScores() const;
		void computeAverageFeatureWeights(const int & round);
	};
}

#endif
