#ifndef _EMPTY_EISNER_2NDF_STATE_H
#define _EMPTY_EISNER_2NDF_STATE_H

#include <unordered_map>

#include "emptyeisner2ndf_macros.h"
#include "common/parser/agenda.h"
#include "include/learning/perceptron/score.h"

namespace emptyeisner2ndf {

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

	class StateItem {
	public:
		int type;
		int left, right;
		ScoreWithSplit jux;
		ScoreWithBiSplit l2r_solid_both, r2l_solid_both, l2r_empty_inside, r2l_empty_inside,
			l2r_solid_outside, r2l_solid_outside;
		ScoreAgenda l2r_empty_outside, r2l_empty_outside;
		ScoreWithSplit l2r, r2l;

	public:

		StateItem();
		~StateItem();

		void init(const int & l, const int & r);

		void updateJUX(const int & split, const tscore & score);
		void updateL2RSolidBoth(const int & split, const int & innersplit, const tscore & score);
		void updateR2LSolidBoth(const int & split, const int & innersplit, const tscore & score);
		void updateL2REmptyOutside(const int & split, const int & innersplit, const tscore & score);
		void updateR2LEmptyOutside(const int & split, const int & innersplit, const tscore & score);
		void updateL2RSolidOutside(const int & split, const int & innersplit, const tscore & score);
		void updateR2LSolidOutside(const int & split, const int & innersplit, const tscore & score);
		void updateL2REmptyInside(const int & split, const int & innersplit, const tscore & score);
		void updateR2LEmptyInside(const int & split, const int & innersplit, const tscore & score);
		void updateL2R(const int & split, const tscore & score);
		void updateR2L(const int & split, const tscore & score);

		void print();
	};

	inline void StateItem::updateJUX(const int & split, const tscore & score) {
		if (jux < score) {
			jux.refer(split, score);
		}
	}

	inline void StateItem::updateL2RSolidBoth(const int & split, const int & innersplit, const tscore & score) {
		if (l2r_solid_both < score) {
			l2r_solid_both.refer(split, innersplit, score);
		}
	}

	inline void StateItem::updateR2LSolidBoth(const int & split, const int & innersplit, const tscore & score) {
		if (r2l_solid_both < score) {
			r2l_solid_both.refer(split, innersplit, score);
		}
	}

	inline void StateItem::updateL2REmptyInside(const int & split, const int & innersplit, const tscore & score) {
		if (l2r_empty_inside < score) {
			l2r_empty_inside.refer(split, innersplit, score);
		}
	}

	inline void StateItem::updateR2LEmptyInside(const int & split, const int & innersplit, const tscore & score) {
		if (r2l_empty_inside < score) {
			r2l_empty_inside.refer(split, innersplit, score);
		}
	}

	inline void StateItem::updateL2RSolidOutside(const int & split, const int & innersplit, const tscore & score) {
		if (l2r_solid_outside < score) {
			l2r_solid_outside.refer(split, innersplit, score);
		}
	}

	inline void StateItem::updateR2LSolidOutside(const int & split, const int & innersplit, const tscore & score) {
		if (r2l_solid_outside < score) {
			r2l_solid_outside.refer(split, innersplit, score);
		}
	}

	inline void StateItem::updateL2REmptyOutside(const int & split, const int & innersplit, const tscore & score) {
		l2r_empty_outside.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	inline void StateItem::updateR2LEmptyOutside(const int & split, const int & innersplit, const tscore & score) {
		r2l_empty_outside.insertItem(ScoreWithBiSplit(split, innersplit, score));
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
