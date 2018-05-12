#include "eisner2nd_state.h"

namespace eisner2nd {
	StateItem::StateItem() = default;
	StateItem::~StateItem() = default;

	void StateItem::init(const int & l, const int & r) {
		type = 0;
		left = l;
		right = r;
		jux.reset();
		l2r_solid_both.reset();
		r2l_solid_both.reset();
		l2r.reset();
		r2l.reset();
	}
}
