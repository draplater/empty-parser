#ifndef _DEPPARSER_BASE_H
#define _DEPPARSER_BASE_H

#include "include/learning/perceptron/score.h"
#include "common/parser/weight_base.h"

enum ParserState {
	TRAIN = 1,
	PARSE,
	GOLDTEST
};

class DepParserBase {
protected:
	int m_nState;

public:
	int m_nTotalErrors;
	int m_nScoreIndex;
	int m_nTrainingRound;

public:
	DepParserBase(int nState) :
		m_nState(nState), m_nTotalErrors(0), m_nScoreIndex(nState == ParserState::TRAIN ? ScoreType::eNonAverage : ScoreType::eAverage), m_nTrainingRound(0) {}
	~DepParserBase() {};
};

#endif
