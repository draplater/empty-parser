#ifndef _EMPTY_EISNER_2NDF_DEPPARSER_H
#define _EMPTY_EISNER_2NDF_DEPPARSER_H

#include <vector>
#include <unordered_set>

#include "emptyeisner2ndf_state.h"
#include "common/parser/depparser_base.h"
#include "common/parser/graph_dp/features/weightec2nd.h"

namespace emptyeisner2ndf {

	class DepParser : public DepParserBase {
	protected:
		float m_fLambda;
		Token m_tWords;
		Token m_tPOSTags;

		static WordPOSTag empty_taggedword;
		static WordPOSTag start_taggedword;
		static WordPOSTag end_taggedword;

		Weightec2nd *m_pWeight;

		StateItem m_lItems[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
		WordPOSTag m_lSentence[MAX_SENTENCE_SIZE][MAX_EMPTYTAG_SIZE];
		WordPOSTag m_lSentenceWithEmpty[MAX_SENTENCE_SIZE];
		std::vector<ECArc> m_vecCorrectECArcs;
		std::vector<ECBiArc> m_vecCorrectBiArcs;
		std::vector<ECArc> m_vecTrainECArcs;
		std::vector<ECBiArc> m_vecTrainBiArcs;
		int m_nSentenceLength;
		int m_nSentenceCount;

		tscore m_lBiSiblingScore[MAX_SENTENCE_SIZE][2][MAX_SENTENCE_SIZE][MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1];

		std::unordered_set<ECArc> m_setArcGoldScore;
		std::unordered_set<ECBiArc> m_setBiSiblingArcGoldScore;

		void update();
		void generate(DependencyTree * retval, const DependencyTree & correct);
		void goldCheck();

		int encodeEmptyWord(int i, int ec);
		int encodeEmptyPOSTag(int i, int ec);
		void readEmptySentAndArcs(const DependencyTree & correct);

		tscore baseArcScore(const int & p, const int & c);
		tscore biSiblingArcScore(const int & p, const int & c, const int & c2);
		virtual void initArcScore(const int & d);
		virtual void initBiSiblingArcScore(const int & d);

		tscore getOrUpdateBaseArcScore(const int & p, const int & c, const int & amount);
		tscore getOrUpdateBiSiblingScore(const int & p, const int & c, const int & c2, const int & amount);

	public:
		DepParser(const std::string & sFeatureInput, const std::string & sFeatureOut, int nState);
		~DepParser();
		Token m_tEmptys;

		int sentenceCount();

		void decode(bool afterscore = false);
		void decodeArcs();
		void decodeSpan(int distance, int left, int right);

		void train(const DependencyTree & correct, const int & round);
		void parse(const Sentence & sentence, DependencyTree * retval);
		void work(DependencyTree * retval, const DependencyTree & correct);

		void initScore(const DependencyTree & tree, float lambda = -1.0);
		void parseScore(const DependencyTree & tree, DependencyTree * retval);
		tscore m_lArcScore[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE][2][MAX_EMPTY_SIZE + 1];

		void finishtraining() {
			m_pWeight->computeAverageFeatureWeights(m_nTrainingRound);
			m_pWeight->saveScores();
			std::cout << "Total number of training errors are: " << m_nTotalErrors << std::endl;
		}
	};
}

#endif
