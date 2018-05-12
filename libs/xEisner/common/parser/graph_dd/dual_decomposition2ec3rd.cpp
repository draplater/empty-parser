#include "dual_decomposition2ec3rd.h"

void DualDecomposition2ec3rd::averageScore(const DependencyTree & tree, bool empty) {
	int treesize = tree.size();
	tscore totalscore = 0;
	for (int id = 0; id < treesize; ++id) {
		const auto & node = tree[id];
		int from = TREENODE_HEAD(node);
		int to = id;
		if (from == -1) from = treesize;
		int x = std::max(from, to);
		int y = std::min(from, to);
		totalscore += empty ? m_pEmptyEisnerSecondParser->m_lArcScore[y][x][y == from ? 0 : 1][0] : m_pEisnerSecondParser->m_lScore[from][to];
	}
	std::cout << "average score : " << (empty ? "(empty)" : "") << (tscore)((tscore)totalscore / (tscore)treesize) << std::endl;
}

void DualDecomposition2ec3rd::init(const std::string & eisner2ndModel, const std::string & emptyeisner2ndModel, tscore step, int maxIter) {
	m_vecSteps.clear();
	m_nMaxIteration = maxIter;
	m_pEisnerSecondParser.reset(new eisner3rd::DepParser(eisner2ndModel, eisner2ndModel, ParserState::PARSE));
	m_pEmptyEisnerSecondParser.reset(new emptyeisner2nd::DepParser(emptyeisner2ndModel, emptyeisner2ndModel, ParserState::PARSE));
	for (int i = 0; i < m_nMaxIteration; ++i) {
		m_vecSteps.push_back(step * (m_nMaxIteration - i) / m_nMaxIteration);
	}
	m_vecOptimalCounts = std::vector<int>(m_nMaxIteration + 1, 0);
}

void DualDecomposition2ec3rd::decode(const DependencyTree &eisner2ndTree, const DependencyTree &emptyeisner2ndTree, DependencyTree &ret_tree) {
	DependencyTree retEisner2ndTree;
	DependencyTree retEmptyEisner2ndTree;
	ret_tree.clear();
	m_pEisnerSecondParser->initScore(eisner2ndTree);
	m_pEmptyEisnerSecondParser->initScore(emptyeisner2ndTree);
	int iter = 0;
	for (; iter < m_nMaxIteration; ++iter) {
		m_pEmptyEisnerSecondParser->parseScore(emptyeisner2ndTree, &retEmptyEisner2ndTree);
		m_pEisnerSecondParser->parseScore(eisner2ndTree, &retEisner2ndTree);
		if (iter == 0 && eisner2ndTree != retEisner2ndTree) {
			std::cout << "fake" << std::endl;
		}
		if (iter == 0 && emptyeisner2ndTree != retEmptyEisner2ndTree) {
			std::cout << emptyeisner2ndTree << std::endl << retEmptyEisner2ndTree << std::endl;
			std::cout << "fake empty" << std::endl;
		}
		if (isCoordinate(retEisner2ndTree, retEmptyEisner2ndTree, iter)) {
			std::cout << "Solution found in iter " << iter << std::endl;
			m_vecOptimalCounts[iter]++;
			ret_tree = retEmptyEisner2ndTree;
			return;
		}
	}
	ret_tree = retEmptyEisner2ndTree;
	std::cout << "Solution found in iter " << iter << std::endl;
	m_vecOptimalCounts[iter]++;
}

bool DualDecomposition2ec3rd::isCoordinate(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, int iteration) {
	bool is_coord = true;
	DependencyTree emptyeisner2ndTreeNonEmpty = emptyToNonEmpty(emptyeisner2ndTree);
	if (emptyeisner2ndTreeNonEmpty.size() != eisner2ndTree.size()) {
		std::cout << "fuck empty convert" << std::endl;
		return false;
	}
	int treesize = eisner2ndTree.size();
	std::vector<bool> tree_arcs_coord(treesize, false);
	tscore gradient = m_vecSteps[iteration];
	//averageScore(eisner2ndTree, false);
	//averageScore(emptyeisner2ndTreeNonEmpty, true);
	for (int id = 0; id < treesize; ++id) {
		const auto & node = eisner2ndTree[id];
		int from = TREENODE_HEAD(node);
		int to = id;
		if (TREENODE_HEAD(emptyeisner2ndTreeNonEmpty[to]) == from) {
			tree_arcs_coord[to] = true;
			continue;
		}
		if (from == -1) from = treesize;
		is_coord = false;
		int x = std::max(from, to);
		int y = std::min(from, to);
		m_pEmptyEisnerSecondParser->m_lArcScore[y][x][y == from ? 0 : 1][0] += gradient;
		m_pEisnerSecondParser->m_lScore[from][to] -= gradient;
	}
	for (int id = 0; id < treesize; ++id) {
		if (!tree_arcs_coord[id]) {
			is_coord = false;
			const auto & node = emptyeisner2ndTreeNonEmpty[id];
			int from = TREENODE_HEAD(node);
			int to = id;
			if (from == -1) from = treesize;
			int x = std::max(from, to);
			int y = std::min(from, to);
			m_pEmptyEisnerSecondParser->m_lArcScore[y][x][y == from ? 0 : 1][0] -= gradient;
			m_pEisnerSecondParser->m_lScore[from][to] += gradient;
		}
	}
	return is_coord;
}