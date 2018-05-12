#ifndef _DUAL_DECOMPOSITION_2ND_H
#define _DUAL_DECOMPOSITION_2ND_H

#include <memory>
#include "common/parser/graph_dp/eisner2nd/eisner2nd_depparser.h"
#include "common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_depparser.h"

class DualDecomposition2nd {
private:
	int m_nMaxIteration;
	int m_nCorrectArc, m_nTotalArc;
	int m_nCorrectECArc, m_nTotalECArc;
	std::vector<tscore> m_vecSteps;
	std::vector<int> m_vecOptimalCounts;
	std::unique_ptr<eisner2nd::DepParser> m_pEisnerSecondParser;
	std::unique_ptr<emptyeisner2nd::DepParser> m_pEmptyEisnerSecondParser;
	void averageScore(const DependencyTree & tree, bool empty);
	bool isCoordinate(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, int iteration);

public:
	DualDecomposition2nd() = default;
	~DualDecomposition2nd() = default;

	void stat();
	void init(const std::string & eisner2ndModel, const std::string & emptyeisner2ndModel, tscore step, int maxIter);
	void decode(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, DependencyTree & ret_tree, float lambda = -1.0);
};

#endif