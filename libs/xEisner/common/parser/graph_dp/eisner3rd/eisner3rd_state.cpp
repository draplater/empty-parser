#include "eisner3rd_state.h"

namespace eisner3rd {
	StateItem::StateItem() = default;
	StateItem::~StateItem() = default;

	void StateItem::init(const int & l, const int & r) {
		type = 0;
		left = l;
		right = r;
		jux.reset();
		l2r_solid_both.clear();
		r2l_solid_both.clear();
		l2r.reset();
		r2l.reset();
	}
}