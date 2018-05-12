#ifndef _EMPTY_EISNER_2ND_STATE_H
#define _EMPTY_EISNER_2ND_STATE_H

#include <unordered_map>

#include "emptyeisner2nd_macros.h"
#include "common/parser/agenda.h"
#include "include/learning/perceptron/score.h"

namespace emptyeisner2nd {

	enum STATE{
		JUX = 0,
		L2R,
		R2L,
		L2R_SOLID_BOTH,
		R2L_SOLID_BOTH,
		L2R_EMPTY_INSIDE,
		R2L_EMPTY_INSIDE,
		L2R_SOLID_OUTSIDE,
		R2L_SOLID_OUTSIDE,
		L2R_EMPTY_OUTSIDE,
		R2L_EMPTY_OUTSIDE = L2R_EMPTY_OUTSIDE + MAX_EMPTY_SIZE,
	};

	extern std::string TYPE_NAME[R2L_EMPTY_OUTSIDE + MAX_EMPTY_SIZE];

	struct StateScore {
		tscore score;
		int split;

		StateScore(tscore sc = 0, int sp = -1) : score(sc), split(sp) {}
		StateScore(const StateScore & ss) : StateScore(ss.score, ss.split) {}

		void reset() { score = 0L; split = -1; }
		void refer(tscore sc, int sp) { score = sc; split = sp; }
		bool operator<(const tscore & sc) const { return split == -1 || score < sc; }
	};

	struct StateItem {
	public:
		int type;
		int left, right;
		StateScore states[R2L_EMPTY_OUTSIDE + MAX_EMPTY_SIZE];

	public:

		StateItem();
		StateItem(const StateItem & item);
		~StateItem();

		void init(const int & l, const int & r)
		{
			left = l;
			right = r;
			type = -1;
			for (int t = 0; t < R2L_EMPTY_OUTSIDE + MAX_EMPTY_SIZE; ++t) {
				states[t].reset();
			}
		}

		void updateStates(const tscore & score, const int & split, int t)
		{ if (states[t] < score) states[t].refer(score, split); }

		void print();
	};
}

#endif
