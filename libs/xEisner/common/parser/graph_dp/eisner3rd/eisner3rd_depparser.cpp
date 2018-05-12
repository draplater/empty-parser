#include <cmath>
#include <stack>
#include <algorithm>
#include <unordered_set>

#include "eisner3rd_depparser.h"

namespace eisner3rd {

	WordPOSTag DepParser::empty_taggedword = WordPOSTag();
	WordPOSTag DepParser::start_taggedword = WordPOSTag();
	WordPOSTag DepParser::end_taggedword = WordPOSTag();

	DepParser::DepParser(const std::string & sFeatureInput, const std::string & sFeatureOut, int nState) :
		DepParserBase(nState), m_tWords(1), m_tPOSTags(1), m_fLambda(-1.0) {

		m_nSentenceLength = 0;

		m_pWeight = new Weight3rd(sFeatureInput, sFeatureOut, m_nScoreIndex, &m_tWords, &m_tPOSTags);

		for (int i = 0; i < MAX_SENTENCE_SIZE; ++i) {
			m_lItems[1][i].init(i, i);
		}

		DepParser::empty_taggedword.refer(m_tWords.lookup(EMPTY_WORD), m_tPOSTags.lookup(EMPTY_POSTAG));
		DepParser::start_taggedword.refer(m_tWords.lookup(START_WORD), m_tPOSTags.lookup(START_POSTAG));
		DepParser::end_taggedword.refer(m_tWords.lookup(END_WORD), m_tPOSTags.lookup(END_POSTAG));

		m_pWeight->init(DepParser::empty_taggedword, DepParser::start_taggedword, DepParser::end_taggedword);
	}

	DepParser::~DepParser() {
		delete m_pWeight;
	}

	void DepParser::train(const DependencyTree & correct, const int & round) {
		// initialize
		int idx = 0;
		m_vecCorrectArcs.clear();
		m_nSentenceLength = correct.size();
		if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) {
			std::cout << "skip one" << std::endl;
			return;
		}
		++m_nTrainingRound;
		// for normal sentence
		for (const auto & node : correct) {
			m_lSentence[idx].refer(m_tWords.lookup(TREENODE_WORD(node)), m_tPOSTags.lookup(TREENODE_POSTAG(node)));
			m_vecCorrectArcs.push_back(Arc(TREENODE_HEAD(node), idx++));
		}
		m_lSentence[idx].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));
		Arcs2TriArcs(m_vecCorrectArcs, m_vecCorrectTriArcs);

		if (m_nState == ParserState::GOLDTEST) {
			m_setFirstGoldScore.clear();
			m_setSecondGoldScore.clear();
			m_setThirdGoldScore.clear();
			for (const auto & triarc : m_vecCorrectTriArcs) {
				m_setFirstGoldScore.insert(BiGram<int>(triarc.first(), triarc.forth()));
				m_setSecondGoldScore.insert(TriGram<int>(triarc.first(), triarc.third(), triarc.forth()));
				m_setThirdGoldScore.insert(QuarGram<int>(triarc.first(), triarc.second(), triarc.third(), triarc.forth()));
			}
		}

		m_pWeight->referRound(m_nTrainingRound);
		work(nullptr, correct);
		if (m_nTrainingRound % OUTPUT_STEP == 0) {
			std::cout << m_nTotalErrors << " / " << m_nTrainingRound << std::endl;
		}
	}

	void DepParser::parse(const Sentence & sentence, DependencyTree * retval) {
		int idx = 0;
		m_nTrainingRound = 0;
		DependencyTree correct;
		m_nSentenceLength = sentence.size();
		if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) {
			std::cout << "skip one" << std::endl;
			return;
		}
		for (const auto & token : sentence) {
			m_lSentence[idx++].refer(m_tWords.lookup(SENT_WORD(token)), m_tPOSTags.lookup(SENT_POSTAG(token)));
			correct.push_back(DependencyTreeNode(token, -1, NULL_LABEL));
		}
		m_lSentence[idx].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));
		work(retval, correct);
	}

	void DepParser::parseScore(const DependencyTree & tree, DependencyTree * retval) {
		decodeAfterScore();
		decodeArcs();
		generate(retval, tree);
	}

	void DepParser::work(DependencyTree * retval, const DependencyTree & correct) {

		decode();

		decodeArcs();

		switch (m_nState) {
		case ParserState::TRAIN:
			update();
			break;
		case ParserState::PARSE:
			generate(retval, correct);
			break;
		case ParserState::GOLDTEST:
			goldCheck();
			break;
		default:
			break;
		}
	}

	void DepParser::decode() {

		for (int d = 2; d <= m_nSentenceLength + 1; ++d) {

			initFirstOrderScore(d);
			initSecondOrderScore(d);

			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {

				int l = i + d - 1;
				StateItem & item = m_lItems[d][i];
				const tscore & l2r_arc_score = m_lFirstOrderScore[ENCODE_L2R(i)];
				const tscore & r2l_arc_score = m_lFirstOrderScore[ENCODE_R2L(i)];

				// initialize
				item.init(i, l);

				// jux
				for (int s = i; s < l; ++s) {
					item.updateJUX(s, m_lItems[s - i + 1][i].l2r.getScore() + m_lItems[l - s][s + 1].r2l.getScore());
				}

				for (int k = i + 1; k < l; ++k) {

					StateItem & litem = m_lItems[k - i + 1][i];
					StateItem & ritem = m_lItems[l - k + 1][k];

					const auto & l_solid_both_beam = litem.l2r_solid_both;
					const auto & r_solid_both_beam = ritem.r2l_solid_both;

					// solid both
					tscore l_base_score = ritem.jux.getScore() + l2r_arc_score + m_lSecondOrderScore[ENCODE_2ND_L2R(i, k)];

					for (const auto & sws : l_solid_both_beam) {
						const int & j = sws->getSplit();
						item.updateL2RSolidBoth(k, j, l_base_score +
							sws->getScore() +
							triArcScore(i, j == i ? -1 : j, k, l));
					}

					tscore r_base_score = litem.jux.getScore() + r2l_arc_score + m_lSecondOrderScore[ENCODE_2ND_R2L(i, k)];

					for (const auto & sws : r_solid_both_beam) {
						const int & j = sws->getSplit();
						item.updateR2LSolidBoth(k, j, r_base_score +
							sws->getScore() +
							triArcScore(l, j == l ? -1 : j, k, i));
					}

					// complete
					item.updateL2R(k, litem.l2r_solid_both.bestItem().getScore() + ritem.l2r.getScore());
					item.updateR2L(k, ritem.r2l_solid_both.bestItem().getScore() + litem.r2l.getScore());
				}
				// solid both
				item.updateL2RSolidBoth(i, i, m_lItems[d - 1][i + 1].r2l.getScore() +
					l2r_arc_score +
					m_lSecondOrderScore[ENCODE_2ND_L2R(i, i)] +
					triArcScore(i, -1, -1, l));
				item.updateR2LSolidBoth(l, l, m_lItems[d - 1][i].l2r.getScore() +
					r2l_arc_score +
					m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] +
					triArcScore(l, -1, -1, i));
				// complete
				item.updateL2R(l, item.l2r_solid_both.bestItem().getScore());
				item.updateR2L(i, item.r2l_solid_both.bestItem().getScore());
			}
			// root
			StateItem & item = m_lItems[d][m_nSentenceLength - d + 1];
			item.init(m_nSentenceLength - d + 1, m_nSentenceLength);
			// solid both
			item.updateR2LSolidBoth(m_nSentenceLength, m_nSentenceLength, m_lItems[d - 1][item.left].l2r.getScore() +
				m_lFirstOrderScore[ENCODE_R2L(item.left)] +
				m_lSecondOrderScore[ENCODE_2ND_R2L(item.left, item.left)] +
				triArcScore(item.right, -1, -1, item.left));
			// complete
			item.updateR2L(item.left, item.r2l_solid_both.bestItem().getScore());
			for (int i = item.left, s = item.left + 1, j = m_nSentenceLength + 1; s < j - 1; ++s) {
				item.updateR2L(s, m_lItems[j - s][s].r2l_solid_both.bestItem().getScore() + m_lItems[s - i + 1][i].r2l.getScore());
			}
		}
	}

	void DepParser::decodeAfterScore() {

		for (int d = 2; d <= m_nSentenceLength + 1; ++d) {

			initSecondOrderScore(d);

			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {

				int l = i + d - 1;
				StateItem & item = m_lItems[d][i];
				const tscore & l2r_arc_score = m_lScore[i][l];
				const tscore & r2l_arc_score = m_lScore[l][i];

				// initialize
				item.init(i, l);

				// jux
				for (int s = i; s < l; ++s) {
					item.updateJUX(s, m_lItems[s - i + 1][i].l2r.getScore() + m_lItems[l - s][s + 1].r2l.getScore());
				}

				for (int k = i + 1; k < l; ++k) {

					StateItem & litem = m_lItems[k - i + 1][i];
					StateItem & ritem = m_lItems[l - k + 1][k];

					const auto & l_solid_both_beam = litem.l2r_solid_both;
					const auto & r_solid_both_beam = ritem.r2l_solid_both;

					// solid both
					tscore l_base_score = ritem.jux.getScore() + l2r_arc_score + m_lSecondOrderScore[ENCODE_2ND_L2R(i, k)];

					for (const auto & sws : l_solid_both_beam) {
						const int & j = sws->getSplit();
						item.updateL2RSolidBoth(k, j, l_base_score +
							sws->getScore() +
							triArcScore(i, j == i ? -1 : j, k, l));
					}

					tscore r_base_score = litem.jux.getScore() + r2l_arc_score + m_lSecondOrderScore[ENCODE_2ND_R2L(i, k)];

					for (const auto & sws : r_solid_both_beam) {
						const int & j = sws->getSplit();
						item.updateR2LSolidBoth(k, j, r_base_score +
							sws->getScore() +
							triArcScore(l, j == l ? -1 : j, k, i));
					}

					// complete
					item.updateL2R(k, litem.l2r_solid_both.bestItem().getScore() + ritem.l2r.getScore());
					item.updateR2L(k, ritem.r2l_solid_both.bestItem().getScore() + litem.r2l.getScore());
				}
				// solid both
				item.updateL2RSolidBoth(i, i, m_lItems[d - 1][i + 1].r2l.getScore() +
					l2r_arc_score +
					m_lSecondOrderScore[ENCODE_2ND_L2R(i, i)] +
					triArcScore(i, -1, -1, l));
				item.updateR2LSolidBoth(l, l, m_lItems[d - 1][i].l2r.getScore() +
					r2l_arc_score +
					m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] +
					triArcScore(l, -1, -1, i));
				// complete
				item.updateL2R(l, item.l2r_solid_both.bestItem().getScore());
				item.updateR2L(i, item.r2l_solid_both.bestItem().getScore());
			}
			// root
			StateItem & item = m_lItems[d][m_nSentenceLength - d + 1];
			item.init(m_nSentenceLength - d + 1, m_nSentenceLength);
			// solid both
			item.updateR2LSolidBoth(m_nSentenceLength, m_nSentenceLength, m_lItems[d - 1][item.left].l2r.getScore() +
				m_lScore[item.right][item.left] +
				m_lSecondOrderScore[ENCODE_2ND_R2L(item.left, item.left)] +
				triArcScore(item.right, -1, -1, item.left));
			// complete
			item.updateR2L(item.left, item.r2l_solid_both.bestItem().getScore());
			for (int i = item.left, s = item.left + 1, j = m_nSentenceLength + 1; s < j - 1; ++s) {
				item.updateR2L(s, m_lItems[j - s][s].r2l_solid_both.bestItem().getScore() + m_lItems[s - i + 1][i].r2l.getScore());
			}
		}
	}

	void DepParser::decodeArcs() {

		m_vecTrainArcs.clear();
		std::stack<std::tuple<int, int, int>> stack;
		m_lItems[m_nSentenceLength + 1][0].type = R2L;
		stack.push(std::tuple<int, int, int>(m_nSentenceLength + 1, -1, 0));

		while (!stack.empty()) {

			auto span = stack.top();
			stack.pop();

			int is = -1;
			int s = std::get<1>(span);
			StateItem & item = m_lItems[std::get<0>(span)][std::get<2>(span)];

			if (item.left == item.right) {
				continue;
			}
			switch (item.type) {
			case JUX:
				if (item.left < item.right - 1) {
					s = item.jux.getSplit();
					m_lItems[s - item.left + 1][item.left].type = L2R;
					stack.push(std::tuple<int, int, int>(s - item.left + 1, -1, item.left));
					m_lItems[item.right - s][s + 1].type = R2L;
					stack.push(std::tuple<int, int, int>(item.right - s, -1, s + 1));
				}
				break;
			case L2R:
				s = item.l2r.getSplit();

				if (item.left < item.right - 1) {
					m_lItems[s - item.left + 1][item.left].type = L2R_SOLID_BOTH;
					stack.push(std::tuple<int, int, int>(s - item.left + 1, m_lItems[s - item.left + 1][item.left].l2r_solid_both.bestItem().getSplit(), item.left));
					m_lItems[item.right - s + 1][s].type = L2R;
					stack.push(std::tuple<int, int, int>(item.right - s + 1, -1, s));
				}
				else {
					m_vecTrainArcs.push_back(BiGram<int>(item.left, item.right));
				}
				break;
			case R2L:
				s = item.r2l.getSplit();

				if (item.left < item.right - 1) {
					m_lItems[item.right - s + 1][s].type = R2L_SOLID_BOTH;
					stack.push(std::tuple<int, int, int>(item.right - s + 1, m_lItems[item.right - s + 1][s].r2l_solid_both.bestItem().getSplit(), s));
					m_lItems[s - item.left + 1][item.left].type = R2L;
					stack.push(std::tuple<int, int, int>(s - item.left + 1, -1, item.left));
				}
				else {
					m_vecTrainArcs.push_back(BiGram<int>(item.right == m_nSentenceLength ? -1 : item.right, item.left));
				}
				break;
			case L2R_SOLID_BOTH:
				m_vecTrainArcs.push_back(BiGram<int>(item.left, item.right));

				if (s == item.left) {
					m_lItems[item.right - s][s + 1].type = R2L;
					stack.push(std::tuple<int, int, int>(item.right - s, -1, s + 1));
				}
				else {
					is = findInnerSplit(item.l2r_solid_both, s);

					m_lItems[s - item.left + 1][item.left].type = L2R_SOLID_BOTH;
					stack.push(std::tuple<int, int, int>(s - item.left + 1, is, item.left));
					m_lItems[item.right - s + 1][s].type = JUX;
					stack.push(std::tuple<int, int, int>(item.right - s + 1, -1, s));
				}
				break;
			case R2L_SOLID_BOTH:
				m_vecTrainArcs.push_back(BiGram<int>(item.right == m_nSentenceLength ? -1 : item.right, item.left));

				if (s == item.right) {
					m_lItems[s - item.left][item.left].type = L2R;
					stack.push(std::tuple<int, int, int>(s - item.left, -1, item.left));
				}
				else {
					is = findInnerSplit(item.r2l_solid_both, s);

					m_lItems[item.right - s + 1][s].type = R2L_SOLID_BOTH;
					stack.push(std::tuple<int, int, int>(item.right - s + 1, is, s));
					m_lItems[s - item.left + 1][item.left].type = JUX;
					stack.push(std::tuple<int, int, int>(s - item.left + 1, -1, item.left));
				}
				break;
			default:
				break;
			}
		}
	}

	void DepParser::update() {
		Arcs2TriArcs(m_vecTrainArcs, m_vecTrainTriArcs);

		std::unordered_set<TriArc> positiveArcs;
		positiveArcs.insert(m_vecCorrectTriArcs.begin(), m_vecCorrectTriArcs.end());
		for (const auto & arc : m_vecTrainTriArcs) {
			positiveArcs.erase(arc);
		}
		std::unordered_set<TriArc> negativeArcs;
		negativeArcs.insert(m_vecTrainTriArcs.begin(), m_vecTrainTriArcs.end());
		for (const auto & arc : m_vecCorrectTriArcs) {
			negativeArcs.erase(arc);
		}
		if (!positiveArcs.empty() || !negativeArcs.empty()) {
			++m_nTotalErrors;
		}
		for (const auto & arc : positiveArcs) {
			getOrUpdateStackScore(arc.first(), arc.second(), arc.third(), arc.forth(), 1);
			getOrUpdateStackScore(arc.first(), arc.third(), arc.forth(), 1);
			getOrUpdateStackScore(arc.first(), arc.forth(), 1);
		}
		for (const auto & arc : negativeArcs) {
			getOrUpdateStackScore(arc.first(), arc.second(), arc.third(), arc.forth(), -1);
			getOrUpdateStackScore(arc.first(), arc.third(), arc.forth(), -1);
			getOrUpdateStackScore(arc.first(), arc.forth(), -1);
		}
	}

	void DepParser::generate(DependencyTree * retval, const DependencyTree & correct) {
		retval->clear();
		for (int i = 0; i < m_nSentenceLength; ++i) {
			retval->push_back(DependencyTreeNode(TREENODE_POSTAGGEDWORD(correct[i]), -1, NULL_LABEL));
		}
		for (const auto & arc : m_vecTrainArcs) {
			TREENODE_HEAD(retval->at(arc.second())) = arc.first();
		}
	}

	void DepParser::goldCheck() {
		Arcs2TriArcs(m_vecTrainArcs, m_vecTrainTriArcs);
		if (m_vecCorrectArcs.size() != m_vecTrainArcs.size() || m_lItems[m_nSentenceLength + 1][0].r2l.getScore() / GOLD_POS_SCORE != 3 * m_vecTrainArcs.size()) {
			std::cout << "gold parse len error at " << m_nTrainingRound << std::endl;
			std::cout << "score is " << m_lItems[m_nSentenceLength + 1][0].r2l.getScore() << std::endl;
			std::cout << "len is " << m_vecTrainArcs.size() << std::endl;
			++m_nTotalErrors;
		}
		else {
			int i = 0;
			std::sort(m_vecCorrectArcs.begin(), m_vecCorrectArcs.end(), [](const Arc & arc1, const Arc & arc2){ return arc1 < arc2; });
			std::sort(m_vecTrainArcs.begin(), m_vecTrainArcs.end(), [](const Arc & arc1, const Arc & arc2){ return arc1 < arc2; });
			for (int n = m_vecCorrectArcs.size(); i < n; ++i) {
				if (m_vecCorrectArcs[i].first() != m_vecTrainArcs[i].first() || m_vecCorrectArcs[i].second() != m_vecTrainArcs[i].second()) {
					break;
				}
			}
			if (i != m_vecCorrectArcs.size()) {
				std::cout << "gold parse tree error at " << m_nTrainingRound << std::endl;
				++m_nTotalErrors;
			}
		}
	}

	tscore DepParser::arcScore(const int & p, const int & c) {
		tscore retval = 0;
		if (m_nState == ParserState::GOLDTEST) {
			retval = m_setFirstGoldScore.find(BiGram<int>(p, c)) == m_setFirstGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return retval;
		}
		retval = getOrUpdateStackScore(p, c, 0);
		if (m_fLambda > 0) retval *= m_fLambda;
		return retval;
	}

	tscore DepParser::twoArcScore(const int & p, const int & c, const int & c2) {
		tscore retval = 0;
		if (m_nState == ParserState::GOLDTEST) {
			retval = m_setSecondGoldScore.find(TriGram<int>(p, c, c2)) == m_setSecondGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return retval;
		}
		retval = getOrUpdateStackScore(p, c, c2, 0);
		if (m_fLambda > 0) retval *= m_fLambda;
		return retval;
	}

	tscore DepParser::triArcScore(const int & p, const int & c, const int & c2, const int & c3) {
		tscore retval = 0;
		if (m_nState == ParserState::GOLDTEST) {
			retval = m_setThirdGoldScore.find(QuarGram<int>(p, c, c2, c3)) == m_setThirdGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return retval;
		}
		retval = getOrUpdateStackScore(p, c, c2, c3, 0);
		if (m_fLambda > 0) retval *= m_fLambda;
		return retval;
	}

	void DepParser::initScore(const DependencyTree & tree, float lambda) {
		m_fLambda = lambda;
		int idx = 0;
		DependencyTree correct;
		m_nSentenceLength = tree.size();

		if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) {
			std::cout << "skip one" << std::endl;
			return;
		}

		for (const auto & token : tree) {
			m_lSentence[idx++].refer(m_tWords.lookup(TREENODE_WORD(token)), m_tPOSTags.lookup(TREENODE_POSTAG(token)));
		}
		m_lSentence[idx].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));

		for (int d = 2; d <= m_nSentenceLength + 1; ++d) {
			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
				m_lScore[i][i + d - 1] = arcScore(i, i + d - 1);
				m_lScore[i + d - 1][i] = arcScore(i + d - 1, i);
			}
			int i = m_nSentenceLength - d + 1;
			m_lScore[m_nSentenceLength][i] = arcScore(m_nSentenceLength, i);
		}
	}

	void DepParser::initFirstOrderScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			m_lFirstOrderScore[ENCODE_L2R(i)] = arcScore(i, i + d - 1);
			m_lFirstOrderScore[ENCODE_R2L(i)] = arcScore(i + d - 1, i);
		}
		int i = m_nSentenceLength - d + 1;
		m_lFirstOrderScore[ENCODE_R2L(i)] = arcScore(m_nSentenceLength, i);
	}

	void DepParser::initSecondOrderScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			int l = i + d - 1;
			m_lSecondOrderScore[ENCODE_2ND_L2R(i, i)] = twoArcScore(i, -1, l);
			m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] = twoArcScore(l, -1, i);
			for (int k = i + 1, l = i + d - 1; k < l; ++k) {
				m_lSecondOrderScore[ENCODE_2ND_L2R(i, k)] = twoArcScore(i, k, l);
				m_lSecondOrderScore[ENCODE_2ND_R2L(i, k)] = twoArcScore(l, k, i);
			}
		}
		int i = m_nSentenceLength - d + 1;
		m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] = twoArcScore(m_nSentenceLength, -1, i);
	}

	tscore DepParser::getOrUpdateStackScore(const int & p, const int & c, const int & amount) {
		return m_pWeight->getOrUpdateArcScore(p, c, amount, m_nSentenceLength, m_lSentence);
	}

	tscore DepParser::getOrUpdateStackScore(const int & p, const int & c, const int & c2, const int & amount) {
		return m_pWeight->getOrUpdateBiArcScore(p, c, c2, amount, m_nSentenceLength, m_lSentence);
	}

	tscore DepParser::getOrUpdateStackScore(const int & p, const int & c, const int & c2, const int & c3, const int & amount) {
		return m_pWeight->getOrUpdateTriArcScore(p, c, c2, c3, amount, m_nSentenceLength, m_lSentence);
	}

}
