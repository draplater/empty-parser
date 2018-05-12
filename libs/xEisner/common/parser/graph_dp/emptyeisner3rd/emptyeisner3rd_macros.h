#ifndef _EMPTY_EISNER_3RD_MACROS_H
#define _EMPTY_EISNER_3RD_MACROS_H

#include <utility>

#include "common/parser/agenda.h"
#include "common/parser/macros_base.h"
#include "include/learning/perceptron/packed_score.h"

namespace emptyeisner3rd {
#define OUTPUT_STEP 10

#define GOLD_POS_SCORE 10
#define GOLD_NEG_SCORE -50

#define AGENDA_SIZE		4
#define MAX_EMPTY_SIZE	1

#define EMPTYTAG			"EMCAT"
#define MAX_EMPTYTAG_SIZE	32

#define ENCODE_L2R(X)			((X) << 1)
#define ENCODE_R2L(X)			(((X) << 1) + 1)
#define ENCODE_2ND_L2R(X,Y)		ENCODE_L2R(((X) << MAX_SENTENCE_BITS) | (Y))
#define ENCODE_2ND_R2L(X,Y)		ENCODE_R2L(((X) << MAX_SENTENCE_BITS) | (Y))

#define ENCODE_EMPTY(X,T)		(((T) << MAX_SENTENCE_BITS) | (X))
#define DECODE_EMPTY_POS(X)		((X) & ((1 << MAX_SENTENCE_BITS) - 1))
#define DECODE_EMPTY_TAG(X)		((X) >> MAX_SENTENCE_BITS)

#define LESS_EMPTY_EMPTY(X,Y)	(DECODE_EMPTY_POS(X) < DECODE_EMPTY_POS(Y))
#define LESS_EMPTY_SOLID(X,Y)	(DECODE_EMPTY_POS(X) <= (Y))
#define LESS_SOLID_EMPTY(X,Y)	((X) < DECODE_EMPTY_POS(Y))
#define LESS_SOLID_SOLID(X,Y)	((X) < (Y))

#define ARC_LEFT(X,Y)			(IS_EMPTY(Y) ? LESS_EMPTY_SOLID(Y,X) : LESS_SOLID_SOLID(Y,X))
#define ARC_RIGHT(X,Y)			(IS_EMPTY(Y) ? LESS_SOLID_EMPTY(X,Y) : LESS_SOLID_SOLID(X,Y))

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

	typedef BiGram<int> Arc;
	typedef QuarGram<int> TriArc;

	typedef AgendaBeam<ScoreWithBiSplit, AGENDA_SIZE> ScoreAgenda;
	typedef AgendaBeam<ScoreWithBiSplit, AGENDA_SIZE << 1> SolidOutsideAgenda;

	bool testTree(const std::vector<Arc> & arcs);
	bool testEmptyTree(const std::vector<Arc> & arcs, const int & len);
	bool operator<(const Arc & arc1, const Arc & arc2);
	bool compareArc(const Arc & arc1, const Arc & arc2);
	int findInnerSplit(const ScoreAgenda & agendaBeam, const int & split);
	void Arcs2TriArcs(std::vector<Arc> & arcs, std::vector<TriArc> & triarcs);
}

#endif