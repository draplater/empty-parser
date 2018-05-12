#ifndef _DUAL_DECOMPOSITION_3EC2ND_H
#define _DUAL_DECOMPOSITION_3EC2ND_H

#include <memory>
#include "common/parser/graph_dp/eisner2nd/eisner2nd_depparser.h"
#include "common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_depparser.h"

class DualDecomposition3ec2nd {
private:
	int m_nMaxIteration;
	std::vector<tscore> m_vecSteps;
	std::vector<int> m_vecOptimalCounts;
	std::unique_ptr<eisner2nd::DepParser> m_pEisnerSecondParser;
	std::unique_ptr<emptyeisner3rd::DepParser> m_pEmptyEisnerSecondParser;
	void averageScore(const DependencyTree & tree, bool empty);
	bool isCoordinate(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, int iteration);

public:
	DualDecomposition3ec2nd() = default;
	~DualDecomposition3ec2nd() = default;

	void init(const std::string & eisner2ndModel, const std::string & emptyeisner2ndModel, tscore step, int maxIter);
	void decode(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, DependencyTree & ret_tree);
};

#endif