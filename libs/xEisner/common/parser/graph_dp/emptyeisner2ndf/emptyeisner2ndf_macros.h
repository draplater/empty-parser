#ifndef _EMPTY_EISNER_2NDF_MACROS_H
#define _EMPTY_EISNER_2NDF_MACROS_H

#include "common/parser/agenda.h"
#include "common/parser/macros_base.h"
#include "include/learning/perceptron/packed_score.h"

namespace emptyeisner2ndf {
#define OUTPUT_STEP 100

#define MAX_EMPTY_SIZE	1
#define MAX_EMPTY_COUNT	16

#define MAX_ACTION_SIZE (9 + (MAX_EMPTY_SIZE << 1))

#define EMPTYTAG			"EMCAT"

#define GOLD_POS_SCORE 10
#define GOLD_NEG_SCORE -50
#define AGENDA_SIZE		4

#define LESS_EMPTY_EMPTY(X,Y)	(DECODE_EMPTY_POS(X) < DECODE_EMPTY_POS(Y))
#define LESS_EMPTY_SOLID(X,Y)	(DECODE_EMPTY_POS(X) <= (Y))
#define LESS_SOLID_EMPTY(X,Y)	((X) < DECODE_EMPTY_POS(Y))
#define LESS_SOLID_SOLID(X,Y)	((X) < (Y))

#define ARC_LEFT(X,Y)			(IS_EMPTY(Y) ? LESS_EMPTY_SOLID(Y,X) : LESS_SOLID_SOLID(Y,X))
#define ARC_RIGHT(X,Y)			(IS_EMPTY(Y) ? LESS_SOLID_EMPTY(X,Y) : LESS_SOLID_SOLID(X,Y))

	typedef BiGram<int> ECArc;

	typedef TriGram<int> ECBiArc;

	typedef AgendaBeam<ScoreWithBiSplit, AGENDA_SIZE> ScoreAgenda;

	bool operator<(const ECArc & arc1, const ECArc & arc2);
	bool compareArc(const ECArc & arc1, const ECArc & arc2);
	void Arcs2BiArcs(std::vector<ECArc> & arcs, std::vector<ECBiArc> & triarcs);

	int findInnerSplit(const ScoreAgenda & agendaBeam, const int & split);
}

#endif
