#ifndef _EMPTY_EISNER_2ND_COMB_DEPPARSER_H
#define _EMPTY_EISNER_2ND_COMB_DEPPARSER_H

#include <vector>
#include <unordered_set>

#include "emptyeisner2nd_state.h"
#include "common/parser/depparser_base.h"
#include "common/parser/graph_dp/features/weight2nd.h"
#include "common/parser/graph_dp/features/weightec2nd.h"

namespace emptyeisner2nd {

	class CombDepParser : public DepParserBase {
	private:
		float m_fLambda;
		Token m_tWords;
		Token m_tPOSTags;
		Token m_tECWords;
		Token m_tECPOSTags;
		Token m_tECEmptys;

		static WordPOSTag empty_taggedword;
		static WordPOSTag start_taggedword;
		static WordPOSTag end_taggedword;

		Weight2nd *m_pWeight;
		Weightec2nd *m_pECWeight;

		StateItem m_lItems[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
		WordPOSTag m_lOriginSentence[MAX_SENTENCE_SIZE];
		WordPOSTag m_lSentence[MAX_SENTENCE_SIZE][MAX_EMPTYTAG_SIZE];
		WordPOSTag m_lSentenceWithEmpty[MAX_SENTENCE_SIZE];
		std::vector<ECArc> m_vecCorrectECArcs;
		std::vector<ECBiArc> m_vecCorrectBiArcs;
		std::vector<ECArc> m_vecTrainECArcs;
		std::vector<ECBiArc> m_vecTrainBiArcs;
		int m_nSentenceLength;
		int m_nSentenceCount;

		tscore m_lArcScore[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][2][MAX_EMPTY_SIZE + 1];
		tscore m_lBiSiblingScore[MAX_SENTENCE_SIZE][2][MAX_SENTENCE_SIZE][MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1];

		std::unordered_set<ECArc> m_setArcGoldScore;
		std::unordered_set<ECBiArc> m_setBiSiblingArcGoldScore;

		void generate(DependencyTree * retval, const DependencyTree & correct);

		int encodeEmptyWord(int i, int ec);
		int encodeEmptyPOSTag(int i, int ec);

		tscore baseArcScore(const int & p, const int & c);
		tscore biSiblingArcScore(const int & p, const int & c, const int & c2);
		void initArcScore(const int & d);
		void initBiSiblingArcScore(const int & d);

		tscore getOrUpdateBaseArcScore(const int & p, const int & c, const int & amount);
		tscore getOrUpdateBiSiblingScore(const int & p, const int & c, const int & c2, const int & amount);

	public:
		CombDepParser(const std::string & sFeatureInput, const std::string & sECFeatureInput, int nState);
		~CombDepParser();

		int sentenceCount();

		void decode(bool afterscore = false);
		void decodeArcs();
		void decodeSpan(int distance, int left, int right);

		void parse(const Sentence & sentence, DependencyTree * retval, float lambda = -1.0);
		void work(DependencyTree * retval, const DependencyTree & correct);
	};
}

#endif
