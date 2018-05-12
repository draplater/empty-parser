#ifndef _DUAL_DECOMPOSITION_3RD_H
#define _DUAL_DECOMPOSITION_3RD_H

#include <memory>
#include "common/parser/graph_dp/eisner3rd/eisner3rd_depparser.h"
#include "common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_depparser.h"

class DualDecomposition3rd {
private:
	int m_nMaxIteration;
	std::vector<tscore> m_vecSteps;
	std::vector<int> m_vecOptimalCounts;
	std::unique_ptr<eisner3rd::DepParser> m_pEisnerThirdParser;
	std::unique_ptr<emptyeisner3rd::DepParser> m_pEmptyEisnerThirdParser;
	void averageScore(const DependencyTree & tree, bool empty);
	bool isCoordinate(const DependencyTree & eisner3rdTree, const DependencyTree & emptyeisner3rdTree, int iteration);

public:
	DualDecomposition3rd() = default;
	~DualDecomposition3rd() = default;

	void init(const std::string & eisner3rdModel, const std::string & emptyeisner3rdModel, tscore step, int maxIter);
	void decode(const DependencyTree & eisner3rdTree, const DependencyTree & emptyeisner3rdTree, DependencyTree & ret_tree, float lambda = -1.0);
};

#endif