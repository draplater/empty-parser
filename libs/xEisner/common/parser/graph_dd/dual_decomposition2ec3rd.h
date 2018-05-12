#ifndef _DUAL_DECOMPOSITION_2EC3RD_H
#define _DUAL_DECOMPOSITION_2EC3RD_H

#include <memory>
#include "common/parser/graph_dp/eisner3rd/eisner3rd_depparser.h"
#include "common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_depparser.h"

class DualDecomposition2ec3rd {
private:
	int m_nMaxIteration;
	std::vector<tscore> m_vecSteps;
	std::vector<int> m_vecOptimalCounts;
	std::unique_ptr<eisner3rd::DepParser> m_pEisnerSecondParser;
	std::unique_ptr<emptyeisner2nd::DepParser> m_pEmptyEisnerSecondParser;
	void averageScore(const DependencyTree & tree, bool empty);
	bool isCoordinate(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, int iteration);

public:
	DualDecomposition2ec3rd() = default;
	~DualDecomposition2ec3rd() = default;

	void init(const std::string & eisner2ndModel, const std::string & emptyeisner2ndModel, tscore step, int maxIter);
	void decode(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, DependencyTree & ret_tree);
};

#endif