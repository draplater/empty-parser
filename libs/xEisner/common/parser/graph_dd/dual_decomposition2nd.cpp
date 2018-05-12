#include "dual_decomposition2nd.h"

void DualDecomposition2nd::averageScore(const DependencyTree & tree, bool empty) {
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

void DualDecomposition2nd::stat() {
	std::cout << "Correctness: " << (float)m_nCorrectArc / (float)m_nTotalArc << std::endl;
	std::cout << "Empty Correctness: " << (float)m_nCorrectECArc / (float)m_nTotalECArc << std::endl;
}

void DualDecomposition2nd::init(const std::string & eisner2ndModel, const std::string & emptyeisner2ndModel, tscore step, int maxIter) {
	m_vecSteps.clear();
	m_nMaxIteration = maxIter;
	m_nCorrectArc = m_nTotalArc = 0;
	m_nCorrectECArc = m_nTotalECArc = 0;
	m_pEisnerSecondParser.reset(new eisner2nd::DepParser(eisner2ndModel, eisner2ndModel, ParserState::PARSE));
	m_pEmptyEisnerSecondParser.reset(new emptyeisner2nd::DepParser(emptyeisner2ndModel, emptyeisner2ndModel, ParserState::PARSE));
	for (int i = 0; i < m_nMaxIteration; ++i) {
		m_vecSteps.push_back(step * (m_nMaxIteration - i) / m_nMaxIteration);
	}
	m_vecOptimalCounts = std::vector<int>(m_nMaxIteration + 1, 0);
}

void DualDecomposition2nd::decode(const DependencyTree &eisner2ndTree, const DependencyTree &emptyeisner2ndTree, DependencyTree &ret_tree, float lambda) {
	DependencyTree retEisner2ndTree;
	DependencyTree retEmptyEisner2ndTree;
	ret_tree.clear();
	m_pEisnerSecondParser->initScore(eisner2ndTree, lambda > 0 ? lambda : -1.0);
	m_pEmptyEisnerSecondParser->initScore(emptyeisner2ndTree, lambda > 0 ? 1.0 - lambda : -1.0);
	int iter = 0;
	for (; iter < m_nMaxIteration; ++iter) {
		m_pEmptyEisnerSecondParser->parseScore(emptyeisner2ndTree, &retEmptyEisner2ndTree);
		m_pEisnerSecondParser->parseScore(eisner2ndTree, &retEisner2ndTree);
		if (iter == 0) {
			m_nTotalArc += retEisner2ndTree.size();
			for (int i = 0, n = retEisner2ndTree.size(); i < n; ++i) {
				if (TREENODE_HEAD(eisner2ndTree[i]) == TREENODE_HEAD(retEisner2ndTree[i])) ++m_nCorrectArc;
			}
			DependencyTree tree1 = emptyToNonEmpty(emptyeisner2ndTree);
			DependencyTree tree2 = emptyToNonEmpty(retEmptyEisner2ndTree);
			m_nTotalECArc += tree2.size();
			for (int i = 0, n = tree2.size(); i < n; ++i) {
				if (TREENODE_HEAD(tree1[i]) == TREENODE_HEAD(tree2[i])) ++m_nCorrectECArc;
			}
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

bool DualDecomposition2nd::isCoordinate(const DependencyTree & eisner2ndTree, const DependencyTree & emptyeisner2ndTree, int iteration) {
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