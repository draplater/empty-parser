#ifndef _EMPTY_EISNER_3RD_DEPPARSER_H
#define _EMPTY_EISNER_3RD_COMB_DEPPARSER_H

#include <vector>
#include <unordered_set>

#include "common/token/token.h"
#include "emptyeisner3rd_state.h"
#include "common/parser/depparser_base.h"
#include "common/parser/graph_dp/features/weight3rd.h"
#include "common/parser/graph_dp/features/weightec3rd.h"

namespace emptyeisner3rd {

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

		Weight3rd *m_pWeight;
		Weightec3rd *m_pECWeight;

		StateItem m_lItems[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
		WordPOSTag m_lOriginSentence[MAX_SENTENCE_SIZE];
		WordPOSTag m_lSentence[MAX_SENTENCE_SIZE][MAX_EMPTYTAG_SIZE];
		std::vector<Arc> m_vecCorrectArcs;
		std::vector<TriArc> m_vecCorrectTriArcs;
		std::vector<Arc> m_vecTrainArcs;
		std::vector<TriArc> m_vecTrainTriArcs;

		int m_nSentenceLength;

		tscore m_lFirstOrderScore[MAX_SENTENCE_SIZE << 1];
		AgendaBeam<ScoreWithType, MAX_EMPTY_SIZE> m_abFirstOrderEmptyScore[MAX_SENTENCE_SIZE << 1];
		tscore m_lSecondOrderScore[(MAX_SENTENCE_SIZE << MAX_SENTENCE_BITS) << 1];

		std::unordered_set<BiGram<int>> m_setFirstGoldScore;
		std::unordered_set<TriGram<int>> m_setSecondGoldScore;
		std::unordered_set<QuarGram<int>> m_setThirdGoldScore;

		void generate(DependencyTree * retval, const DependencyTree & correct);

		int encodeEmptyWord(int i, int ec);
		int encodeEmptyPOSTag(int i, int ec);

		tscore arcScore(const int & p, const int & c);
		tscore twoArcScore(const int & p, const int & c, const int & c2);
		tscore triArcScore(const int & p, const int & c, const int & c2, const int & c3);
		void initFirstOrderScore(const int & d);
		void initSecondOrderScore(const int & d);

		tscore getOrUpdateStackScore(const int & p, const int & c, const int & amount);
		tscore getOrUpdateStackScore(const int & p, const int & c, const int & c2, const int & amount);
		tscore getOrUpdateStackScore(const int & p, const int & c, const int & c2, const int & c3, const int & amount);


	public:
		CombDepParser(const std::string & sFeatureInput, const std::string & sECFeatureInput, int nState);
		~CombDepParser();

		void decode();
		void decodeArcs();

		void parse(const Sentence & sentence, DependencyTree * retval, float lambda = -1.0);
		void work(DependencyTree * retval, const DependencyTree & correct);
	};
}

#endif
