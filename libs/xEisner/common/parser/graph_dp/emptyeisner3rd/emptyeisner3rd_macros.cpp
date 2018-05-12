#include <stack>
#include <algorithm>
#include <unordered_set>

#include "emptyeisner3rd_macros.h"

namespace emptyeisner3rd {

	bool testEmptyTree(const std::vector<Arc> & arcs, const int & len) {
		std::vector<Arc> nonEmptyArcs;
		for (const auto & arc : arcs) {
			if (IS_EMPTY(arc.first())) {
				std::cout << "empty head" << std::endl;
				return false;
			}
			if (IS_EMPTY(arc.second())) {
				if (arc.first() == -1) {
					std::cout << "root has empty child" << std::endl;
					return false;
				}
			}
			else {
				nonEmptyArcs.push_back(arc);
			}
		}
		if (nonEmptyArcs.size() != len) {
			std::cout << "non empty arcs len error" << std::endl;
			return false;
		}
		return testTree(nonEmptyArcs);
	}

	bool testTree(const std::vector<Arc> & arcs) {
		int l = arcs.size();
		std::unordered_set<int> preds;
		std::vector<int> heads;
		std::vector<bool> flags;
		for (int i = 0; i < l; ++i) {
			heads.push_back(-2);
			flags.push_back(false);
		}
		int root = -1;
		for (const auto & arc : arcs) {
			heads[arc.second()] = arc.first();
			preds.insert(arc.second());
			if (arc.first() == -1) {
				if (root != -1) {
					std::cout << "multiple root" << std::endl;
					return false;
				}
				root = arc.second();
			}
		}
		if (root == -1) {
			std::cout << "no root" << std::endl;
			return false;
		}
		std::stack<int> stack;
		stack.push(root);
		flags[root] = true;
		while (!stack.empty()) {
			int node = stack.top();
			stack.pop();
			for (int i = 0; i < l; ++i) {
				if (heads[i] == node) {
					if (flags[i]) {
						std::cout << "has circle" << std::endl;
						return false;
					}
					flags[i] = true;
					stack.push(i);
				}
			}
		}
		for (const auto & p : preds) {
			if (!flags[p]) {
				std::cout << p << " no head" << std::endl;
				return false;
			}
		}
		return true;
	}

	bool operator<(const Arc & arc1, const Arc & arc2) {
		if (arc1.first() != arc2.first()) {
			return arc1.first() < arc2.first();
		}
		if (IS_EMPTY(arc1.second())) {
			if (IS_EMPTY(arc2.second())) {
				return LESS_EMPTY_EMPTY(arc1.second(), arc2.second());
			}
			else {
				return LESS_EMPTY_SOLID(arc1.second(), arc2.second());
			}
		}
		else {
			if (IS_EMPTY(arc2.second())) {
				return LESS_SOLID_EMPTY(arc1.second(), arc2.second());
			}
			else {
				return LESS_SOLID_SOLID(arc1.second(), arc2.second());
			}
		}
	}

	bool compareArc(const Arc & arc1, const Arc & arc2) {
		if (DECODE_EMPTY_POS(arc1.second()) != DECODE_EMPTY_POS(arc2.second())) {
			return DECODE_EMPTY_POS(arc1.second()) < DECODE_EMPTY_POS(arc2.second());
		}
		if (ARC_LEFT(arc1.first(), arc1.second())) {
			if (ARC_RIGHT(arc2.first(), arc2.second())) {
				return false;
			}
			else {
				return arc1.first() > arc2.first();
			}
		}
		else {
			if (ARC_LEFT(arc2.first(), arc2.second())) {
				return true;
			}
			else {
				return arc1.first() > arc2.first();
			}
		}
	}

	bool operator<(const ScoreWithType & swt1, const ScoreWithType & swt2) {
		return swt1.getScore() < swt2.getScore();
	}

	int findInnerSplit(const ScoreAgenda & agendaBeam, const int & split) {
		int is = -1;
		for (const auto & agenda : agendaBeam) {
			if (agenda->getSplit() == split) {
				is = agenda->getInnerSplit();
				break;
			}
		}
		if (is == -1) {
			std::cout << "bad eisner at " << __LINE__ << std::endl;
		}
		return is;
	}

	void Arcs2TriArcs(std::vector<Arc> & arcs, std::vector<TriArc> & triarcs) {
		int maxid = -1;
		for (auto & arc : arcs) {
			if (!IS_EMPTY(arc.second()) && maxid < arc.second()) {
				maxid = arc.second();
			}
		}
		for (auto & arc : arcs) {
			if (arc.first() == -1) {
				arc.refer(maxid + 1, arc.second());
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
			int head = itr_s->first();
			if (itr_e - itr_s > 1) {
				for (auto itr = itr_s; itr != itr_e; ++itr) {
					if (ARC_LEFT(head, itr->second())) {
						if (itr == itr_e - 1 || ARC_RIGHT(head, (itr + 1)->second())) {
							triarcs.push_back(TriArc(head, -1, -1, itr->second()));
						}
						else if (itr == itr_e - 2 || ARC_RIGHT(head, (itr + 2)->second())) {
							triarcs.push_back(TriArc(head, -1, (itr + 1)->second(), itr->second()));
						}
						else {
							triarcs.push_back(TriArc(head, (itr + 2)->second(), (itr + 1)->second(), itr->second()));
						}
					}
					else {
						if (itr == itr_s || ARC_LEFT(head, (itr - 1)->second())) {
							triarcs.push_back(TriArc(head, -1, -1, itr->second()));
						}
						else if (itr == itr_s + 1 || ARC_LEFT(head, (itr - 2)->second())) {
							triarcs.push_back(TriArc(head, -1, (itr - 1)->second(), itr->second()));
						}
						else {
							triarcs.push_back(TriArc(head, (itr - 2)->second(), (itr - 1)->second(), itr->second()));
						}
					}
				}
			}
			else {
				triarcs.push_back(TriArc(head, -1, -1, itr_s->second()));
			}
			itr_s = itr_e;
		}
		for (auto & arc : arcs) {
			if (arc.first() == arcs.size()) {
				arc.refer(-1, arc.second());
				break;
			}
		}
		if (triarcs.size() != arcs.size()) {
			std::cout << "tri transform error" << std::endl;
		}
	}
}