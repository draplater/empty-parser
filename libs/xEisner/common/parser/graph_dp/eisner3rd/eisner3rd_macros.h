#ifndef _EISNER3RD_MACROS_H
#define _EISNER3RD_MACROS_H

#include "common/parser/agenda.h"
#include "common/parser/macros_base.h"
#include "include/learning/perceptron/packed_score.h"

namespace eisner3rd {
#define OUTPUT_STEP 100

#define AGENDA_SIZE 4

#define GOLD_POS_SCORE 10
#define GOLD_NEG_SCORE -50

#define ENCODE_L2R(X)			((X) << 1)
#define ENCODE_R2L(X)			(((X) << 1) + 1)
#define ENCODE_2ND_L2R(X,Y)		ENCODE_L2R(((X) << MAX_SENTENCE_BITS) | (Y))
#define ENCODE_2ND_R2L(X,Y)		ENCODE_R2L(((X) << MAX_SENTENCE_BITS) | (Y))

	typedef PackedScoreMap<WordInt> WordIntMap;
	typedef PackedScoreMap<POSTagInt> POSTagIntMap;

	typedef PackedScoreMap<TwoWordsInt> TwoWordsIntMap;
	typedef PackedScoreMap<POSTagSet2Int> POSTagSet2IntMap;
	typedef PackedScoreMap<WordPOSTagInt> WordPOSTagIntMap;

	typedef PackedScoreMap<POSTagSet3Int> POSTagSet3IntMap;
	typedef PackedScoreMap<WordWordPOSTagInt> WordWordPOSTagIntMap;
	typedef PackedScoreMap<WordPOSTagPOSTagInt> WordPOSTagPOSTagIntMap;

	typedef PackedScoreMap<POSTagSet4Int> POSTagSet4IntMap;
	typedef PackedScoreMap<WordWordPOSTagPOSTagInt> WordWordPOSTagPOSTagIntMap;
	typedef PackedScoreMap<WordPOSTagPOSTagPOSTagInt> WordPOSTagPOSTagPOSTagIntMap;

	typedef AgendaBeam<ScoreWithBiSplit, AGENDA_SIZE> ScoreAgenda;

	typedef BiGram<int> Arc;
	typedef QuarGram<int> TriArc;

	bool operator<(const Arc & arc1, const Arc & arc2);
	int findInnerSplit(const ScoreAgenda & agendaBeam, const int & split);
	void Arcs2TriArcs(std::vector<Arc> & arcs, std::vector<TriArc> & triarcs);
}

#endif