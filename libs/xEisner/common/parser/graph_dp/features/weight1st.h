#ifndef _WEIGHT1ST_H
#define _WEIGHT1ST_H

#include "common/token/token.h"
#include "common/parser/macros_base.h"
#include "common/parser/weight_base.h"
#include "include/learning/perceptron/packed_score.h"

class Weight1st : public WeightBase {
protected:

	Token * m_pUnusedWords;
	Token * m_pUnusedPOSTags;

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

	WordPOSTag m_tkStart, m_tkEnd, m_tkEmpty;
	int m_nScoreIndex, m_nTrainingRound;

public:
	Weight1st(const std::string & sRead, const std::string & sRecord, int nParserState, Token * words, Token * postags);
	~Weight1st();

	void loadScores();
	void saveScores() const;
	void computeAverageFeatureWeights(const int & round);

	void referRound(const int & nRound);
	void init(const WordPOSTag & tkEmpty, const WordPOSTag & tkStart, const WordPOSTag & tkEnd);
	tscore getOrUpdateArcScore(const int & p, const int & c, const int & amount, int sentLen, WordPOSTag (&sent)[MAX_SENTENCE_SIZE]);
};

#endif
