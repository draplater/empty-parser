#include "dual_decomposition3rd.h"

void DualDecomposition3rd::averageScore(const DependencyTree & tree, bool empty) {
	int treesize = tree.size();
	tscore totalscore = 0;
	for (int id = 0; id < treesize; ++id) {
		const auto & node = tree[id];
		int from = TREENODE_HEAD(node);
		int to = id;
		if (from == -1) from = treesize;
		int x = std::max(from, to);
		int y = std::min(from, to);
		totalscore += empty ? m_pEmptyEisnerThirdParser->m_lScore[y][x][y == from ? 0 : 1] : m_pEisnerThirdParser->m_lScore[from][to];
	}
	std::cout << "average score : " << (empty ? "(empty)" : "") << (tscore)((tscore)totalscore / (tscore)treesize) << std::endl;
}

void DualDecomposition3rd::init(const std::string & eisner3rdModel, const std::string & emptyeisner3rdModel, tscore step, int maxIter) {
	m_vecSteps.clear();
	m_nMaxIteration = maxIter;
	m_pEisnerThirdParser.reset(new eisner3rd::DepParser(eisner3rdModel, eisner3rdModel, ParserState::PARSE));
	m_pEmptyEisnerThirdParser.reset(new emptyeisner3rd::DepParser(emptyeisner3rdModel, emptyeisner3rdModel, ParserState::PARSE));
	for (int i = 0; i < m_nMaxIteration; ++i) {
		m_vecSteps.push_back(step * (m_nMaxIteration - i) / m_nMaxIteration);
	}
	m_vecOptimalCounts = std::vector<int>(m_nMaxIteration + 1, 0);
}

void DualDecomposition3rd::decode(const DependencyTree &eisner3rdTree, const DependencyTree &emptyeisner3rdTree, DependencyTree &ret_tree, float lambda) {
	DependencyTree retEisner3rdTree;
	DependencyTree retEmptyEisner3rdTree;
	ret_tree.clear();
	Sentence sent;
	for (const auto & node : eisner3rdTree) {
		sent.push_back(TREENODE_POSTAGGEDWORD(node));
	}
	m_pEisnerThirdParser->initScore(eisner3rdTree, lambda > 0 ? lambda : -1.0);
	m_pEmptyEisnerThirdParser->initScore(emptyeisner3rdTree, lambda > 0 ? 1.0 - lambda : -1.0);
	//m_pEmptyEisnerThirdParser->parse(sent, &retEmptyEisner3rdTree);
	//if (emptyeisner3rdTree != retEmptyEisner3rdTree) {
	//	std::cout << emptyeisner3rdTree << std::endl << retEmptyEisner3rdTree << std::endl;
	//	std::cout << "fake parse" << std::endl;
	//}
	int iter = 0;
	for (; iter < m_nMaxIteration; ++iter) {
		m_pEmptyEisnerThirdParser->parseScore(emptyeisner3rdTree, &retEmptyEisner3rdTree);
		//std::cout << emptyeisner3rdTree << std::endl << retEmptyEisner3rdTree << std::endl;
		m_pEisnerThirdParser->parseScore(eisner3rdTree, &retEisner3rdTree);
		//std::cout << eisner3rdTree << std::endl << retEisner3rdTree << std::endl;
		if (isCoordinate(retEisner3rdTree, retEmptyEisner3rdTree, iter)) {
			std::cout << "Solution found in iter " << iter << std::endl;
			m_vecOptimalCounts[iter]++;
			ret_tree = retEmptyEisner3rdTree;
			return;
		}
	}
	ret_tree = retEmptyEisner3rdTree;
	std::cout << "Solution found in iter " << iter << std::endl;
	m_vecOptimalCounts[iter]++;
}

bool DualDecomposition3rd::isCoordinate(const DependencyTree & eisner3rdTree, const DependencyTree & emptyeisner3rdTree, int iteration) {
	bool is_coord = true;
	DependencyTree emptyeisner3rdTreeNonEmpty = emptyToNonEmpty(emptyeisner3rdTree);
	if (emptyeisner3rdTreeNonEmpty.size() != eisner3rdTree.size()) {
		std::cout << "fuck empty convert" << std::endl;
		std::cout << emptyeisner3rdTreeNonEmpty << std::endl;
		std::cout << eisner3rdTree << std::endl;
		return false;
	}
	int treesize = eisner3rdTree.size();
	std::vector<bool> tree_arcs_coord(treesize, false);
	tscore gradient = m_vecSteps[iteration];
	//averageScore(eisner3rdTree, false);
	//averageScore(emptyeisner3rdTreeNonEmpty, true);
	for (int id = 0; id < treesize; ++id) {
		const auto & node = eisner3rdTree[id];
		int from = TREENODE_HEAD(node);
		int to = id;
		if (TREENODE_HEAD(emptyeisner3rdTreeNonEmpty[to]) == from) {
			tree_arcs_coord[to] = true;
			continue;
		}
		if (from == -1) from = treesize;
		is_coord = false;
		int x = std::max(from, to);
		int y = std::min(from, to);
		m_pEmptyEisnerThirdParser->m_lScore[y][x][y == from ? 0 : 1] += gradient;
		m_pEisnerThirdParser->m_lScore[from][to] -= gradient;
	}
	for (int id = 0; id < treesize; ++id) {
		if (!tree_arcs_coord[id]) {
			is_coord = false;
			const auto & node = emptyeisner3rdTreeNonEmpty[id];
			int from = TREENODE_HEAD(node);
			int to = id;
			if (from == -1) from = treesize;
			int x = std::max(from, to);
			int y = std::min(from, to);
			m_pEmptyEisnerThirdParser->m_lScore[y][x][y == from ? 0 : 1] -= gradient;
			m_pEisnerThirdParser->m_lScore[from][to] += gradient;
		}
	}
	return is_coord;
}