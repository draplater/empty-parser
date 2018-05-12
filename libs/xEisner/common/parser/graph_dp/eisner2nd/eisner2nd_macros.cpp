#include <stack>
#include <algorithm>
#include <unordered_set>

#include "eisner2nd_macros.h"

namespace eisner2nd {

	bool operator<(const Arc & arc1, const Arc & arc2) {
		if (arc1.first() != arc2.first()) {
			return arc1.first() < arc2.first();
		}
		return arc1.second() < arc2.second();
	}

	void Arcs2BiArcs(std::vector<Arc> & arcs, std::vector<BiArc> & triarcs) {
		for (auto & arc : arcs) {
			if (arc.first() == -1) {
				arc.refer(arcs.size(), arc.second());
				break;
			}
		}
		triarcs.clear();
		std::sort(arcs.begin(), arcs.end(), [](const Arc & arc1, const Arc & arc2) { return arc1 < arc2; });
		auto itr_s = arcs.begin(), itr_e = arcs.begin();
		while (itr_s != arcs.end()) {
			while (itr_e != arcs.end() && itr_e->first() == itr_s->first()) {
				++itr_e;
			}
			if (itr_e - itr_s > 1) {
				int head = itr_s->first();
				for (auto itr = itr_s; itr != itr_e; ++itr) {
					if (head > itr->second()) {
						if (itr == itr_e - 1 || head < (itr + 1)->second()) {
							triarcs.push_back(BiArc(head, -1, itr->second()));
						}
						else {
							triarcs.push_back(BiArc(head, (itr + 1)->second(), itr->second()));
						}
					}
					else {
						if (itr == itr_s || head > (itr - 1)->second()) {
							triarcs.push_back(BiArc(head, -1, itr->second()));
						}
						else {
							triarcs.push_back(BiArc(head, (itr - 1)->second(), itr->second()));
						}
					}
				}
			}
			else {
				triarcs.push_back(BiArc(itr_s->first(), -1, itr_s->second()));
			}
			itr_s = itr_e;
		}
		for (auto & arc : arcs) {
			if (arc.first() == arcs.size()) {
				arc.refer(-1, arc.second());
				break;
			}
		}
	}
}

extern "C" {
int arcs_to_biarcs(int length, int arcs[][2], int biarcs[][3]) {
	using namespace eisner2nd;
	std::vector<Arc> arcs_v;
	std::vector<BiArc> biarcs_v;
	for(int i=0; i<length; i++) {
		arcs_v.emplace_back(arcs[i][0], arcs[i][1]);
	}
	Arcs2BiArcs(arcs_v, biarcs_v);
	for(int i=0; i<length; i++) {
		auto& biarc = biarcs_v[i];
        biarcs[i][0] = biarc.first();
        biarcs[i][1] = biarc.second();
		biarcs[i][2] = biarc.third();
    }
	return static_cast<int>(biarcs_v.size());
}
};
