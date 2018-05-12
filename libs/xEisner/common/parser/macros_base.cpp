#include <map>
#include <set>
#include <stack>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "macros_base.h"

int encodeLinkDistance(const int & st, const int & n0) {
	int diff = n0 - st;
	if (diff > 10) {
		diff = 6;
	}
	else if (diff > 5) {
		diff = 5;
	}
	return diff;
}

int encodeEmptyDistance(const int & hi, const int & di) {
	int diff = hi - di;
	if (diff < 0) return diff < -1 ? -2 : -1;
	return diff > 10 ? 11 : diff;
}

int encodeLinkDistanceOrDirection(const int & hi, const int & di, bool dir) {
	int diff = hi - di;
	if (dir) {
		return diff > 0 ? 7 : -7;
	}
	if (diff < 0) {
		diff = -diff;
	}
	if (diff > 10) {
		diff = 6;
	}
	else if (diff > 5) {
		diff = 5;
	}
	if (hi < di) {
		diff = -diff;
	}
	return diff;
}

std::string nCharPrev(const Sentence & sent, int index, int n) {
	std::string str = "";
	--index;
	while (index >= 0 && n > 0) {
		const std::string & word = SENT_WORD(sent[index]);
		int i = word.length() - 1;
		while (i >= 0) {
			const unsigned char & c = word[i];
			if ((c >> 6) != 2) {
				--n;
				if (n == 0) {
					break;
				}
			}
			--i;
		}
		str = word.substr(i == -1 ? 0 : i) + str;
		--index;
	}
	while (n--) {
		str = "(P)" + str;
	}
	return str;
}

std::string nCharNext(const Sentence & sent, int index, int n) {
	std::string str = "";
	++index;
	while (index < sent.size() && n > 0) {
		const std::string & word = SENT_WORD(sent[index]);
		int i = 1;
		while (i < word.length()) {
			const unsigned char & c = word[i];
			if ((c >> 6) != 2) {
				--n;
				if (n == 0) {
					break;
				}
			}
			++i;
		}
		if (i == word.length()) {
			--n;
		}
		str = str + word.substr(0, i);
		++index;
	}
	while (n--) {
		str = str + "(N)";
	}
	return str;
}

std::istream & operator>>(std::istream & input, Sentence & sentence) {
	sentence.clear();
	ttoken line, token;
	std::getline(input, line);
	std::istringstream iss(line);
	while (iss >> token) {
		int i = token.rfind(SENT_SPTOKEN);
		sentence.push_back(POSTaggedWord(token.substr(0, i), token.substr(i + 1)));
	}
	return input;
}

std::istream & operator>>(std::istream & input, DependencyTree & tree) {
	tree.clear();
	ttoken line, token;
	while (true) {
		std::getline(input, line);
		if (line.empty()) {
			break;
		}
		DependencyTreeNode node;
		std::istringstream iss(line);
		iss >> TREENODE_WORD(node) >> TREENODE_POSTAG(node) >> TREENODE_HEAD(node) >> TREENODE_LABEL(node);
		tree.push_back(node);
	}
	return input;
}

std::istream & operator>>(std::istream & input, DependencyTaggedTree & tree) {
	tree.clear();
	ttoken line, token;
	while (true) {
		std::getline(input, line);
		if (line.empty()) {
			break;
		}
		DependencyTaggedTreeNode node;
		std::istringstream iss(line);
		iss >> TREENODE_WORD(node.first) >> TREENODE_POSTAG(node.first) >> TREENODE_HEAD(node.first) >> TREENODE_LABEL(node.first) >> node.second;
		tree.push_back(node);
	}
	return input;
}

std::ostream & operator<<(std::ostream & output, const Sentence & sentence) {
	auto itr = sentence.begin();
	while (true) {
		output << SENT_WORD(*itr) << SENT_SPTOKEN << SENT_POSTAG(*itr);
		if (++itr == sentence.end()) {
			break;
		}
		else {
			output << " ";
		}
	}
	output << std::endl;
	return output;
}

std::ostream & operator<<(std::ostream & output, const DependencyTree & tree) {
	for (auto itr = tree.begin(); itr != tree.end(); ++itr) {
		output << TREENODE_WORD(*itr) << "\t" << TREENODE_POSTAG(*itr) << "\t" << TREENODE_HEAD(*itr) << "\t" << TREENODE_LABEL(*itr) << std::endl;
	}
	output << std::endl;
	return output;
}

std::ostream & operator<<(std::ostream & output, const DependencyTaggedTree & tree) {
	for (auto itr = tree.begin(); itr != tree.end(); ++itr) {
		output << TREENODE_WORD(itr->first) << "\t" << TREENODE_POSTAG(itr->first) << "\t" << TREENODE_HEAD(itr->first) << "\t" << TREENODE_LABEL(itr->first) << "\t" << itr->second << std::endl;
	}
	output << std::endl;
	return output;
}

void nBackSpace(const std::string & str) {
	for (int i = 0; i < str.size(); ++i) {
		std::cout << '\b';
	}
	std::cout << std::flush;
}

bool hasNonProjectiveTree(std::set<std::pair<int, int>> goldArcs, int len) {
	typedef std::pair<int, int> Arc;
	typedef std::tuple<int, int> Sws;
	typedef std::tuple<Sws, Sws, Sws, Sws> State;
	std::vector<std::vector<State>> vecStates;
	vecStates.push_back(std::vector<State>());
	std::vector<State> uniStates;
	for (int i = 0; i < len; ++i) {
		uniStates.push_back(State(Sws(i, 0), Sws(i, 0), Sws(i, 0), Sws(i, 0)));
	}
	vecStates.push_back(uniStates);
	for (int d = 2; d <= len + 1; ++d) {

		std::vector<State> levelStates;

		for (int i = 0, n = len - d + 1; i < n; ++i) {

			State item(Sws(-1, -len - 1), Sws(-1, -len - 1), Sws(-1, -len - 1), Sws(-1, -len - 1));

			const int & l2r_arc_score = goldArcs.find(Arc(i, i + d - 1)) == goldArcs.end() ? -1 : 1;
			const int & r2l_arc_score = goldArcs.find(Arc(i + d - 1, i)) == goldArcs.end() ? -1 : 1;

			for (int s = i, j = i + d; s < j - 1; ++s) {

				int partial_im_complete_score = std::get<1>(std::get<0>(vecStates[s - i + 1][i])) + std::get<1>(std::get<1>(vecStates[j - s - 1][s + 1]));

				if (std::get<1>(std::get<2>(item)) < partial_im_complete_score + l2r_arc_score) {
					std::get<0>(std::get<2>(item)) = s;
					std::get<1>(std::get<2>(item)) = partial_im_complete_score + l2r_arc_score;
				}
				if (std::get<1>(std::get<3>(item)) < partial_im_complete_score + r2l_arc_score) {
					std::get<0>(std::get<3>(item)) = s;
					std::get<1>(std::get<3>(item)) = partial_im_complete_score + r2l_arc_score;
				}
			}

			if (std::get<1>(std::get<0>(item)) < std::get<1>(std::get<2>(item))) {
				std::get<0>(std::get<0>(item)) = i + d - 1;
				std::get<1>(std::get<0>(item)) = std::get<1>(std::get<2>(item));
			}
			if (std::get<1>(std::get<1>(item)) < std::get<1>(std::get<3>(item))) {
				std::get<0>(std::get<1>(item)) = i;
				std::get<1>(std::get<1>(item)) = std::get<1>(std::get<3>(item));
			}

			for (int s = i + 1, j = i + d; s < j - 1; ++s) {

				State & litem = vecStates[s - i + 1][i];
				State & ritem = vecStates[j - s][s];

				if (std::get<1>(std::get<0>(item)) < std::get<1>(std::get<2>(litem)) + std::get<1>(std::get<0>(ritem))) {
					std::get<0>(std::get<0>(item)) = s;
					std::get<1>(std::get<0>(item)) = std::get<1>(std::get<2>(litem)) + std::get<1>(std::get<0>(ritem));
				}
				if (std::get<1>(std::get<1>(item)) < std::get<1>(std::get<3>(ritem)) + std::get<1>(std::get<1>(litem))) {
					std::get<0>(std::get<1>(item)) = s;
					std::get<1>(std::get<1>(item)) = std::get<1>(std::get<3>(ritem)) + std::get<1>(std::get<1>(litem));
				}
			}

			levelStates.push_back(item);
		}

		State item(Sws(-1, -len - 1), Sws(-1, -len - 1), Sws(-1, -len - 1), Sws(-1, -len - 1));

		std::get<0>(std::get<3>(item)) = len - 1;
		std::get<1>(std::get<3>(item)) = (goldArcs.find(Arc(len, len - d + 1)) == goldArcs.end() ? -1 : 1) + std::get<1>(std::get<0>(vecStates[d - 1][len - d + 1]));

		std::get<0>(std::get<1>(item)) = len - d + 1;
		std::get<1>(std::get<1>(item)) = std::get<1>(std::get<3>(item));

		for (int i = len - d + 1, s = i + 1, j = len + 1; s < j - 1; ++s) {
			if (std::get<1>(std::get<1>(item)) < std::get<1>(std::get<3>(vecStates[j - s][s])) + std::get<1>(std::get<1>(vecStates[s - i + 1][i]))) {
				std::get<0>(std::get<1>(item)) = s;
				std::get<1>(std::get<1>(item)) = std::get<1>(std::get<3>(vecStates[j - s][s])) + std::get<1>(std::get<1>(vecStates[s - i + 1][i]));
			}
		}

		levelStates.push_back(item);

		vecStates.push_back(levelStates);
	}
	return std::get<1>(std::get<1>(vecStates[len + 1][0])) == len;
}

DependencyTree emptyToNonEmpty(const DependencyTree & tree) {
	DependencyTree ret_tree;
	// for normal sentence
	std::vector<int> tIds;
	int m_nSentenceLength = 0;
	for (const auto & node : tree) {
		int h = TREENODE_HEAD(node), p = tIds.size();
		if (TREENODE_POSTAG(node) != "EMCAT") {
			tIds.push_back(m_nSentenceLength++);
			ret_tree.push_back(node);
		}
		else {
			tIds.push_back(-1);
		}
		if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) return ret_tree;
	}
	for (auto && node : ret_tree) {
		if (TREENODE_HEAD(node) != -1) TREENODE_HEAD(node) = tIds[TREENODE_HEAD(node)];
	}
	return ret_tree;
}
