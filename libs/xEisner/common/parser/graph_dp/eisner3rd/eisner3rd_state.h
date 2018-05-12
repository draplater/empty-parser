#ifndef _EISNER3RD_STATE_H
#define _EISNER3RD_STATE_H

#include "eisner3rd_macros.h"
#include "common/parser/agenda.h"
#include "include/learning/perceptron/score.h"

namespace eisner3rd {

	enum STATE{
		JUX = 1,
		L2R_SOLID_BOTH,
		R2L_SOLID_BOTH,
		L2R,
		R2L
	};

	class StateItem {
	public:
		int type;
		int left, right;
		ScoreWithSplit jux;
		ScoreAgenda l2r_solid_both, r2l_solid_both;
		ScoreWithSplit l2r, r2l;

	public:

		StateItem();
		~StateItem();

		void init(const int & l, const int & r);

		void updateJUX(const int & split, const tscore & score);
		void updateL2RSolidBoth(const int & split, const int & innersplit, const tscore & score);
		void updateR2LSolidBoth(const int & split, const int & innersplit, const tscore & score);
		void updateL2R(const int & split, const tscore & score);
		void updateR2L(const int & split, const tscore & score);
	};

	inline void StateItem::updateJUX(const int & split, const tscore & score) {
		if (jux < score) {
			jux.refer(split, score);
		}
	}

	inline void StateItem::updateL2RSolidBoth(const int & split, const int & innersplit, const tscore & score) {
		l2r_solid_both.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	inline void StateItem::updateR2LSolidBoth(const int & split, const int & innersplit, const tscore & score) {
		r2l_solid_both.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	inline void StateItem::updateL2R(const int & split, const tscore & score) {
		if (l2r < score) {
			l2r.refer(split, score);
		}
	}

	inline void StateItem::updateR2L(const int & split, const tscore & score) {
		if (r2l < score) {
			r2l.refer(split, score);
		}
	}
}

#endif
