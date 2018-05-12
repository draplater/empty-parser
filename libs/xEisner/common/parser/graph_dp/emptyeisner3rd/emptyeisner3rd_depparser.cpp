#include <cmath>
#include <stack>
#include <algorithm>
#include <unordered_set>

#include "emptyeisner3rd_depparser.h"

namespace emptyeisner3rd {

	WordPOSTag DepParser::empty_taggedword = WordPOSTag();
	WordPOSTag DepParser::start_taggedword = WordPOSTag();
	WordPOSTag DepParser::end_taggedword = WordPOSTag();

	DepParser::DepParser(const std::string & sFeatureInput, const std::string & sFeatureOut, int nState) :
		DepParserBase(nState), m_tWords(1), m_tPOSTags(1), m_tEmptys(1), m_fLambda(-1.0) {

		m_nSentenceLength = 0;

		m_pWeight = new Weightec3rd(sFeatureInput, sFeatureOut, m_nScoreIndex, &m_tWords, &m_tPOSTags, &m_tEmptys);

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
		m_vecCorrectArcs.clear();

		readEmptySentAndArcs(correct);
		if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) {
			std::cout << "skip one" << std::endl;
			return;
		}
		++m_nTrainingRound;

		if (!testEmptyTree(m_vecCorrectArcs, m_nSentenceLength)) {
			std::cout << m_nTrainingRound << std::endl;
			std::cout << "tree error" << std::endl;
			std::cout << "arcs" << std::endl;
			for (const auto & arc : m_vecCorrectArcs) {
				std::cout << arc.first() << " " << arc.second() << std::endl;
			}
		}

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
			m_lSentence[idx][0].refer(m_tWords.lookup(SENT_WORD(token)), m_tPOSTags.lookup(SENT_POSTAG(token)));
			for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
				m_lSentence[idx][ec].refer(
					m_tWords.lookup(
					(idx == 0 ? START_WORD : m_tWords.key(m_lSentence[idx - 1][0].first())) +
						m_tEmptys.key(ec) +
						SENT_WORD(token)
					),
					m_tPOSTags.lookup("NN")
				);
			}
			correct.push_back(DependencyTreeNode(token, -1, NULL_LABEL));
			++idx;
		}
		m_lSentence[idx][0].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));
		for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
			m_lSentence[idx][ec].refer(
				m_tWords.lookup(
					m_tWords.key(m_lSentence[idx - 1][0].first()) + m_tEmptys.key(ec) + END_WORD
				),
				m_tPOSTags.lookup("NN")
			);
		}
		work(retval, correct);
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
		for (int d = 1; d <= m_nSentenceLength + 1; ++d) {
			initFirstOrderScore(d);
			if (d > 1) {
				initSecondOrderScore(d);
			}
			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
				int l = i + d - 1;
				tscore l2r_arc_score = m_lFirstOrderScore[ENCODE_L2R(i)];
				tscore r2l_arc_score = m_lFirstOrderScore[ENCODE_R2L(i)];
				StateItem & item = m_lItems[d][i];

				// initialize
				item.init(i, l);

				for (int s = i; s < l; ++s) {
					const StateItem & litem = m_lItems[s - i + 1][i];
					const StateItem & ritem = m_lItems[l - s][s + 1];
					// jux
					item.updateJUX(s, litem.l2r.getScore() + ritem.r2l.getScore());
					// l2r_empty_inside
					tscore l_empty_inside_base_score = l2r_arc_score + ritem.r2l.getScore();
					for (const auto & l_beam : litem.l2r_empty_outside) {
						int p = ENCODE_EMPTY(s, DECODE_EMPTY_TAG(l_beam->getSplit()));
						int k = ENCODE_EMPTY(s + 1, DECODE_EMPTY_TAG(l_beam->getSplit()));
						int j = DECODE_EMPTY_POS(l_beam->getSplit());
						item.updateL2REmptyInside(p, l_beam->getSplit(), l_empty_inside_base_score +
							l_beam->getScore() +
							twoArcScore(i, k, l) +
							triArcScore(i, j == i ? -1 : j, k, l));
						//std::cout << "(1)" << std::endl;
						//std::cout << i << " -> " << k << " with " << l << " : " << twoArcScore(i, k, l) << std::endl;
						//std::cout << i << " -> " << (j == i ? -1 : j) << " with " << k << " with " << l << " : " << triArcScore(i, j == i ? -1 : j, k, l) << std::endl;
					}
					// r2l_empty_inside
					tscore r_empty_inside_base_score = r2l_arc_score + litem.l2r.getScore();
					for (const auto & r_beam : ritem.r2l_empty_outside) {
						int p = ENCODE_EMPTY(s, DECODE_EMPTY_TAG(r_beam->getSplit()));
						int k = ENCODE_EMPTY(s + 1, DECODE_EMPTY_TAG(r_beam->getSplit()));
						int j = DECODE_EMPTY_POS(r_beam->getSplit());
						item.updateR2LEmptyInside(p, r_beam->getSplit(), r_empty_inside_base_score +
							r_beam->getScore() +
							twoArcScore(l, k, i) +
							triArcScore(l, j == l ? -1 : j, k, i));
						//std::cout << "(2)" << std::endl;
						//std::cout << l << " -> " << k << " with " << i << " : " << twoArcScore(l, k, i) << std::endl;
						//std::cout << l << " -> " << (j == l ? -1 : j) << " with " << k << " with " << i << " : " << triArcScore(l, j == l ? -1 : j, k, i) << std::endl;
					}
				}
				for (int k = i + 1; k < l; ++k) {
					StateItem & litem = m_lItems[k - i + 1][i];
					StateItem & ritem = m_lItems[l - k + 1][k];

					const auto & l_beam = litem.l2r_solid_outside;
					const auto & r_beam = ritem.r2l_solid_outside;

					// l2r_solid_both
					tscore l_solid_both_base_score = ritem.jux.getScore() + l2r_arc_score + m_lSecondOrderScore[ENCODE_2ND_L2R(i, k)];
					for (const auto & swbs : l_beam) {
						// j is inner split
						const int & j = swbs->getSplit();
						item.updateL2RSolidBoth(k, j, l_solid_both_base_score +
							swbs->getScore() +
							triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, l));
						//std::cout << "(3)" << std::endl;
						//std::cout << i << " -> " << (j == i ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << l << " : " << triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, l) << std::endl;
					}
					// l2r_empty_outside
					for (const auto & swt : m_abFirstOrderEmptyScore[ENCODE_L2R(i)]) {
						const int & t = swt->getType();
						// s is split with type
						int s = ENCODE_EMPTY(k, t);
						// o is outside empty
						int o = ENCODE_EMPTY(l + 1, t);
						tscore l_empty_outside_base_score = ritem.l2r.getScore() + swt->getScore() + twoArcScore(i, k, o);
						for (const auto & swbs : l_beam) {
							// j is inner split
							const int & j = swbs->getSplit();
							item.updateL2REmptyOutside(s, j, l_empty_outside_base_score +
								swbs->getScore() +
								triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o));
							//std::cout << "(4)" << std::endl;
							//std::cout << i << " -> " << (j == i ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << o << " : " << triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o) << std::endl;
						}
					}
					// l2r
					item.updateL2R(k, litem.l2r_solid_outside.bestItem().getScore() + ritem.l2r.getScore());
					// r2l_solid_both
					tscore r_solid_both_base_score = litem.jux.getScore() + r2l_arc_score + m_lSecondOrderScore[ENCODE_2ND_R2L(i, k)];
					for (const auto & swbs : r_beam) {
						const int & j = swbs->getSplit();
						item.updateR2LSolidBoth(k, j, r_solid_both_base_score +
							swbs->getScore() +
							triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, i));
						//std::cout << "(5)" << std::endl;
						//std::cout << l << " -> " << (j == l ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << i << " : " << triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, i) << std::endl;
					}
					// r2l_empty_outside
					for (const auto & swt : m_abFirstOrderEmptyScore[ENCODE_R2L(i)]) {
						const int & t = swt->getType();
						int s = ENCODE_EMPTY(k, t);
						int o = ENCODE_EMPTY(i, t);
						tscore r_empty_outside_base_score = litem.r2l.getScore() + swt->getScore() + twoArcScore(l, k, o);
						for (const auto & swbs : r_beam) {
							const int & j = swbs->getSplit();
							item.updateR2LEmptyOutside(s, j, r_empty_outside_base_score +
								swbs->getScore() +
								triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o));
							//std::cout << "(6)" << std::endl;
							//std::cout << l << " -> " << (j == l ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << o << " : " << triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o) << std::endl;
						}
					}
					// r2l
					item.updateR2L(k, ritem.r2l_solid_outside.bestItem().getScore() + litem.r2l.getScore());
				}
				if (d > 1) {
					// l2r_solid_both
					item.updateL2RSolidBoth(i, i, m_lItems[d - 1][i + 1].r2l.getScore() +
						l2r_arc_score +
						m_lSecondOrderScore[ENCODE_2ND_L2R(i, i)] +
						triArcScore(i, -1, -1, l));
					//std::cout << "(7)" << std::endl;
					//std::cout << i << " -> " << -1 << " with " << -1 << " with " << l << " : " << triArcScore(i, -1, -1, l) << std::endl;
					// r2l_solid_both
					item.updateR2LSolidBoth(l, l, m_lItems[d - 1][i].l2r.getScore() +
						r2l_arc_score +
						m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] +
						triArcScore(l, -1, -1, i));
					//std::cout << "(8)" << std::endl;
					//std::cout << l << " -> " << -1 << " with " << -1 << " with " << i << " : " << triArcScore(l, -1, -1, i) << std::endl;
					// l2r_solid_outside
					for (const auto & swbs : item.l2r_empty_inside) {
						item.updateL2RSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
					for (const auto & swbs : item.l2r_solid_both) {
						item.updateL2RSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
					// r2l_solid_outside
					for (const auto & swbs : item.r2l_empty_inside) {
						item.updateR2LSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
					for (const auto & swbs : item.r2l_solid_both) {
						item.updateR2LSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
				}
				// l2r_empty_ouside
				for (const auto & swt : m_abFirstOrderEmptyScore[ENCODE_L2R(i)]) {
					const int & t = swt->getType();
					int s = ENCODE_EMPTY(l, t);
					int o = ENCODE_EMPTY(l + 1, t);
					if (d > 1) {
						tscore base_l2r_empty_outside_score = swt->getScore() + twoArcScore(i, l, o) + m_lItems[1][l].l2r.getScore();
						for (const auto & swbs : item.l2r_solid_outside) {
							const int & j = swbs->getSplit();
							//std::cout << "score " << swbs->getScore() << std::endl;
							item.updateL2REmptyOutside(s, swbs->getSplit(), base_l2r_empty_outside_score +
								swbs->getScore() +
								triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, l, o));
							//std::cout << "(9)" << std::endl;
							//std::cout << i << " -> " << (j == i ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << l << " with " << o << " : " << triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, l, o) << std::endl;
						}
					}
					else {
						item.updateL2REmptyOutside(s, i, swt->getScore() +
							twoArcScore(i, -1, o) +
							triArcScore(i, -1, -1, o));
						//std::cout << "(10)" << std::endl;
						//std::cout << i << " -> " << -1 << " with " << o << twoArcScore(i, -1, o) << std::endl;
						//std::cout << i << " -> " << -1 << " with " << -1 << " with " << o << " : " << triArcScore(i, -1, -1, o) << std::endl;
					}
				}
				// r2l_empty_outside
				for (const auto & swt : m_abFirstOrderEmptyScore[ENCODE_R2L(i)]) {
					const int & t = swt->getType();
					int s = ENCODE_EMPTY(i, t);
					int o = ENCODE_EMPTY(i, t);
					if (d > 1) {
						tscore base_r2l_empty_outside_score = swt->getScore() + twoArcScore(l, i, o) + m_lItems[1][i].r2l.getScore();
						for (const auto & swbs : item.r2l_solid_outside) {
							const int & j = swbs->getSplit();
							item.updateR2LEmptyOutside(s, swbs->getSplit(), base_r2l_empty_outside_score +
								swbs->getScore() +
								triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, i, o));
							//std::cout << "(11)" << std::endl;
							//std::cout << l << " -> " << (j == l ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << i << " with " << o << " : " << triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, i, o) << std::endl;
						}
					}
					else {
						item.updateR2LEmptyOutside(s, l, swt->getScore() +
							twoArcScore(l, -1, o) +
							triArcScore(l, -1, -1, o));
						//std::cout << "(12)" << std::endl;
						//std::cout << l << " -> " << -1 << " with " << i << twoArcScore(l, -1, o) << std::endl;
						//std::cout << l << " -> " << -1 << " with " << -1 << " with " << o << " : " << triArcScore(l, -1, -1, o) << std::endl;
					}

				}
				// l2r
				item.updateL2R(l, item.l2r_solid_outside.bestItem().getScore() + m_lItems[1][l].l2r.getScore());
				if (item.l2r_empty_outside.size() > 0) {
					item.updateL2R(item.l2r_empty_outside.bestItem().getSplit(), item.l2r_empty_outside.bestItem().getScore());
				}
				// r2l
				item.updateR2L(i, item.r2l_solid_outside.bestItem().getScore() + m_lItems[1][i].r2l.getScore());
				if (item.r2l_empty_outside.size() > 0) {
					item.updateR2L(item.r2l_empty_outside.bestItem().getSplit(), item.r2l_empty_outside.bestItem().getScore());
				}

			}
			if (d > 1) {
				// root
				StateItem & item = m_lItems[d][m_nSentenceLength - d + 1];
				item.init(m_nSentenceLength - d + 1, m_nSentenceLength);
				// r2l_solid_outside
				item.updateR2LSolidOutside(m_nSentenceLength, m_nSentenceLength, m_lItems[d - 1][item.left].l2r.getScore() +
					m_lFirstOrderScore[ENCODE_R2L(item.left)] +
					m_lSecondOrderScore[ENCODE_2ND_R2L(item.left, item.left)] +
					triArcScore(item.right, -1, -1, item.left));
				//std::cout << "(13)" << std::endl;
				//std::cout << item.right << " -> -1 with -1 with " << item.left << " : " << triArcScore(item.right, -1, -1, item.left) << std::endl;
				// r2l
				item.updateR2L(item.left, item.r2l_solid_outside.bestItem().getScore() + m_lItems[1][item.left].r2l.getScore());
				for (int i = item.left, s = item.left + 1, j = m_nSentenceLength + 1; s < j - 1; ++s) {
					item.updateR2L(s, m_lItems[j - s][s].r2l_solid_outside.bestItem().getScore() + m_lItems[s - i + 1][i].r2l.getScore());
				}

			}
		}
	}

	void DepParser::decodeArcs() {
		m_vecTrainArcs.clear();
		std::stack<std::tuple<int, int, int, int>> stack;
		stack.push(std::tuple<int, int, int, int>(m_nSentenceLength + 1, -1, 0, R2L));
		m_lItems[m_nSentenceLength + 1][0].type = R2L;

		while (!stack.empty()) {
			std::tuple<int, int, int, int> span = stack.top();
			stack.pop();
			StateItem & item = m_lItems[std::get<0>(span)][std::get<2>(span)];
			item.type = std::get<3>(span);
			int split = std::get<1>(span);
			int innersplit = -1;
			//item.print();

			switch (item.type) {
			case JUX:
				split = item.jux.getSplit();

				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, L2R));
				stack.push(std::tuple<int, int, int, int>(item.right - split, -1, split + 1, R2L));
				break;
			case L2R:
				split = item.l2r.getSplit();

				if (IS_EMPTY(split)) {
					std::get<1>(span) = item.l2r_empty_outside.bestItem().getSplit();
					std::get<3>(span) = L2R_EMPTY_OUTSIDE;
					stack.push(span);
					break;
				}
				if (item.left == item.right) {
					break;
				}

				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, m_lItems[split - item.left + 1][item.left].l2r_solid_outside.bestItem().getSplit(), item.left, L2R_SOLID_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(item.right - split + 1, -1, split, L2R));
				break;
			case R2L:
				split = item.r2l.getSplit();

				if (IS_EMPTY(split)) {
					std::get<1>(span) = item.r2l_empty_outside.bestItem().getSplit();
					std::get<3>(span) = R2L_EMPTY_OUTSIDE;
					stack.push(span);
					break;
				}
				if (item.left == item.right) {
					break;
				}

				stack.push(std::tuple<int, int, int, int>(item.right - split + 1, m_lItems[item.right - split + 1][split].r2l_solid_outside.bestItem().getSplit(), split, R2L_SOLID_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, R2L));
				break;
			case L2R_SOLID_BOTH:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainArcs.push_back(BiGram<int>(item.left, item.right));

				if (split == item.left) {
					stack.push(std::tuple<int, int, int, int>(item.right - split, -1, split + 1, R2L));
				}
				else {
					item.l2r_solid_both.sortItems();
					innersplit = findInnerSplit(item.l2r_solid_both, split);

					stack.push(std::tuple<int, int, int, int>(split - item.left + 1, innersplit, item.left, L2R_SOLID_OUTSIDE));
					stack.push(std::tuple<int, int, int, int>(item.right - split + 1, -1, split, JUX));
				}
				break;
			case R2L_SOLID_BOTH:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainArcs.push_back(BiGram<int>(item.right == m_nSentenceLength ? -1 : item.right, item.left));

				if (split == item.right) {
					stack.push(std::tuple<int, int, int, int>(split - item.left, -1, item.left, L2R));
				}
				else {
					item.r2l_solid_both.sortItems();
					innersplit = findInnerSplit(item.r2l_solid_both, split);

					stack.push(std::tuple<int, int, int, int>(item.right - split + 1, innersplit, split, R2L_SOLID_OUTSIDE));
					stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, JUX));
				}
				break;
			case L2R_EMPTY_INSIDE:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainArcs.push_back(BiGram<int>(item.left, item.right));

				item.l2r_empty_inside.sortItems();
				innersplit = findInnerSplit(item.l2r_empty_inside, split);
				split = DECODE_EMPTY_POS(split);

				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, innersplit, item.left, L2R_EMPTY_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(item.right - split, -1, split + 1, R2L));
				break;
			case R2L_EMPTY_INSIDE:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainArcs.push_back(BiGram<int>(item.right, item.left));

				item.r2l_empty_inside.sortItems();
				innersplit = findInnerSplit(item.r2l_empty_inside, split);
				split = DECODE_EMPTY_POS(split);

				stack.push(std::tuple<int, int, int, int>(item.right - split, innersplit, split + 1, R2L_EMPTY_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, L2R));
				break;
			case L2R_EMPTY_OUTSIDE:
				m_vecTrainArcs.push_back(BiGram<int>(item.left, ENCODE_EMPTY(item.right + 1, DECODE_EMPTY_TAG(split))));

				if (item.left == item.right) {
					break;
				}

				innersplit = findInnerSplit(item.l2r_empty_outside, split);
				split = DECODE_EMPTY_POS(split);

				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, innersplit, item.left, L2R_SOLID_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(item.right - split + 1, -1, split, L2R));
				break;
			case R2L_EMPTY_OUTSIDE:
				m_vecTrainArcs.push_back(BiGram<int>(item.right, ENCODE_EMPTY(item.left, DECODE_EMPTY_TAG(split))));

				if (item.left == item.right) {
					break;
				}

				innersplit = findInnerSplit(item.r2l_empty_outside, split);
				split = DECODE_EMPTY_POS(split);

				stack.push(std::tuple<int, int, int, int>(item.right - split + 1, innersplit, split, R2L_SOLID_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, R2L));
				break;
			case L2R_SOLID_OUTSIDE:
				if (item.left == item.right) {
					break;
				}

				std::get<3>(span) = IS_EMPTY(split) ? L2R_EMPTY_INSIDE : L2R_SOLID_BOTH;
				stack.push(span);
				break;
			case R2L_SOLID_OUTSIDE:
				if (item.left == item.right) {
					break;
				}
				if (item.right == m_nSentenceLength) {
					m_vecTrainArcs.push_back(BiGram<int>(-1, item.left));
					stack.push(std::tuple<int, int, int, int>(item.right - item.left, -1, item.left, L2R));
					break;
				}

				std::get<3>(span) = IS_EMPTY(split) ? R2L_EMPTY_INSIDE : R2L_SOLID_BOTH;
				stack.push(span);
				break;
			default:
				break;
			}
		}

		if (!testEmptyTree(m_vecTrainArcs, m_nSentenceLength)) {
			if (m_vecTrainArcs.size() != m_nSentenceLength) {
				std::cout << "len error" << std::endl;
				std::cout << "correct len is " << m_vecCorrectArcs.size() << std::endl;
				std::cout << "train len is " << m_vecTrainArcs.size() << std::endl;
			}
			else {
				std::cout << "tree error" << std::endl;
			}
			std::cout << "train arcs" << std::endl;
			for (const auto & arc : m_vecTrainArcs) {
				std::cout << "from " << arc.first() << " to " << arc.second() << std::endl;
			}
			std::cout << "correct arcs" << std::endl;
			for (const auto & arc : m_vecCorrectArcs) {
				std::cout << "from " << arc.first() << " to " << arc.second() << std::endl;
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
		else {
			for (const auto & arc : m_vecTrainArcs) {
				if (IS_EMPTY(arc.second())) {
					std::cout << "generate an empty edge at " << m_nTrainingRound << std::endl;
					break;
				}
			}
		}
		//std::cout << "positive" << std::endl;	//debug
		for (const auto & arc : positiveArcs) {
			//std::cout << arc << std::endl;	//debug
			getOrUpdateStackScore(arc.first(), arc.second(), arc.third(), arc.forth(), 1);
			getOrUpdateStackScore(arc.first(), arc.third(), arc.forth(), 1);
			getOrUpdateStackScore(arc.first(), arc.forth(), 1);
		}
		//std::cout << "negative" << std::endl;	//debug
		for (const auto & arc : negativeArcs) {
			//std::cout << arc << std::endl;	//debug
			getOrUpdateStackScore(arc.first(), arc.second(), arc.third(), arc.forth(), -1);
			getOrUpdateStackScore(arc.first(), arc.third(), arc.forth(), -1);
			getOrUpdateStackScore(arc.first(), arc.forth(), -1);
		}
	}

	void DepParser::generate(DependencyTree * retval, const DependencyTree & correct) {
		retval->clear();
		std::vector<Arc> emptyArcs;
		for (const auto & arc : m_vecTrainArcs) {
			if (IS_EMPTY(arc.second())) {
				emptyArcs.push_back(arc);
			}
		}
		std::sort(emptyArcs.begin(), emptyArcs.end(), [](const Arc & arc1, const Arc & arc2) { return compareArc(arc1, arc2); });

		auto itr_e = emptyArcs.begin();
		int idx = 0, nidx = 0;
		std::unordered_map<int, int> nidmap;
		nidmap[-1] = -1;
		while (itr_e != emptyArcs.end() && idx != m_nSentenceLength) {
			if (idx < DECODE_EMPTY_POS(itr_e->second())) {
				nidmap[idx] = nidx++;
				retval->push_back(DependencyTreeNode(TREENODE_POSTAGGEDWORD(correct[idx++]), -1, NULL_LABEL));
			}
			else {
				nidmap[(itr_e->second() << MAX_SENTENCE_BITS) + itr_e->first()] = nidx++;
				retval->push_back(DependencyTreeNode(POSTaggedWord(m_tEmptys.key(DECODE_EMPTY_TAG((itr_e++)->second())), EMPTYTAG), -1, NULL_LABEL));
			}
		}
		while (idx != m_nSentenceLength) {
			nidmap[idx] = nidx++;
			retval->push_back(DependencyTreeNode(TREENODE_POSTAGGEDWORD(correct[idx++]), -1, NULL_LABEL));
		}
		while (itr_e != emptyArcs.end()) {
			nidmap[(itr_e->second() << MAX_SENTENCE_BITS) + itr_e->first()] = nidx++;
			retval->push_back(DependencyTreeNode(POSTaggedWord(m_tEmptys.key(DECODE_EMPTY_TAG((itr_e++)->second())), EMPTYTAG), -1, NULL_LABEL));
		}

		for (const auto & arc : m_vecTrainArcs) {
			TREENODE_HEAD(retval->at(nidmap[IS_EMPTY(arc.second()) ? ((arc.second() << MAX_SENTENCE_BITS) + arc.first()) : arc.second()])) = nidmap[arc.first()];
		}
	}

	void DepParser::goldCheck() {
		Arcs2TriArcs(m_vecTrainArcs, m_vecTrainTriArcs);
		if (m_vecCorrectArcs.size() != m_vecTrainArcs.size() || m_lItems[m_nSentenceLength + 1][0].r2l.getScore() / GOLD_POS_SCORE != 3 * m_vecTrainArcs.size()) {
			++m_nTotalErrors;
			std::cout << "gold parse len error at " << m_nTrainingRound << std::endl;
			for (const auto & triarc : m_vecTrainTriArcs) {
				std::cout << triarc.first() << " " << triarc.second() << " " << triarc.third() << " " << triarc.forth() << std::endl;
			}
			std::cout << std::endl;
		}
		else {
			int i = 0;
			std::sort(m_vecCorrectArcs.begin(), m_vecCorrectArcs.end(), [](const Arc & arc1, const Arc & arc2) { return arc1 < arc2; });
			std::sort(m_vecTrainArcs.begin(), m_vecTrainArcs.end(), [](const Arc & arc1, const Arc & arc2) { return arc1 < arc2; });
			for (int n = m_vecCorrectArcs.size(); i < n; ++i) {
				if (m_vecCorrectArcs[i].first() != m_vecTrainArcs[i].first() || m_vecCorrectArcs[i].second() != m_vecTrainArcs[i].second()) {
					++m_nTotalErrors;
					break;
				}
			}
			if (i != m_vecCorrectArcs.size()) {
				std::cout << "gold parse tree error at " << m_nTrainingRound << std::endl;
				for (const auto & triarc : m_vecTrainTriArcs) {
					std::cout << triarc.first() << " " << triarc.second() << " " << triarc.third() << " " << triarc.forth() << std::endl;
				}
				std::cout << std::endl;
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
		//		std::cout << p << " -> " << c << " : " << m_nRetval << std::endl;
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
		//		std::cout << p << " -> " << c << " with " << c2 << " : " << m_nRetval << std::endl;
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
		//		std::cout << retval << " -> " << c << " with " << c2 << " with " << c3 << " : " << m_nRetval << std::endl;
		return retval;
	}

	void DepParser::initFirstOrderScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			int l = i + d - 1;
			if (d > 1) {
				m_lFirstOrderScore[ENCODE_L2R(i)] = arcScore(i, i + d - 1);
				m_lFirstOrderScore[ENCODE_R2L(i)] = arcScore(i + d - 1, i);
				//std::cout << i << " -> " << l << " : " << m_lFirstOrderScore[ENCODE_L2R(i)] << std::endl;
				//std::cout << l << " -> " << i << " : " << m_lFirstOrderScore[ENCODE_R2L(i)] << std::endl;
			}
			m_abFirstOrderEmptyScore[ENCODE_L2R(i)].clear();
			m_abFirstOrderEmptyScore[ENCODE_R2L(i)].clear();

			for (int t = 1; t <= MAX_EMPTY_SIZE; ++t) {
				if (m_nState == ParserState::GOLDTEST || m_pWeight->testEmptyNode(i, ENCODE_EMPTY(i + d, t), m_lSentence)) {
					const tscore & score = arcScore(i, ENCODE_EMPTY(i + d, t));
					m_abFirstOrderEmptyScore[ENCODE_L2R(i)].insertItem(ScoreWithType(t, score));
				}
				if (m_nState == ParserState::GOLDTEST || m_pWeight->testEmptyNode(i + d - 1, ENCODE_EMPTY(i, t), m_lSentence)) {
					const tscore & score = arcScore(i + d - 1, ENCODE_EMPTY(i, t));
					m_abFirstOrderEmptyScore[ENCODE_R2L(i)].insertItem(ScoreWithType(t, score));
				}
			}
			m_abFirstOrderEmptyScore[ENCODE_L2R(i)].sortItems();
			m_abFirstOrderEmptyScore[ENCODE_R2L(i)].sortItems();
			//for (int i = 0; i < m_abFirstOrderEmptyScore[ENCODE_L2R(i)].size(); ++i) {
			//	std::cout << i << " -> " << l << " : " << m_abFirstOrderEmptyScore[ENCODE_L2R(i)].bestItem(0).getScore() << std::endl;
			//}
			//for (int i = 0; i < m_abFirstOrderEmptyScore[ENCODE_R2L(i)].size(); ++i) {
			//	std::cout << l << " -> " << i << " : " << m_abFirstOrderEmptyScore[ENCODE_R2L(i)].bestItem(0).getScore() << std::endl;
			//}
		}
		int i = m_nSentenceLength - d + 1;
		m_lFirstOrderScore[ENCODE_R2L(i)] = arcScore(m_nSentenceLength, i);
		//std::cout << m_nSentenceLength << " -> " << i << " : " << m_lFirstOrderScore[ENCODE_R2L(i)] << std::endl;
	}

	void DepParser::initSecondOrderScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			int l = i + d - 1;
			m_lSecondOrderScore[ENCODE_2ND_L2R(i, i)] = twoArcScore(i, -1, l);
			m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] = twoArcScore(l, -1, i);
			//std::cout << i << " -> " << -1 << " with " << l << " : " << m_lSecondOrderScore[ENCODE_2ND_L2R(i, i)] << std::endl;
			//std::cout << l << " -> " << -1 << " with " << i << " : " << m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] << std::endl;
			for (int k = i + 1, l = i + d - 1; k < l; ++k) {
				m_lSecondOrderScore[ENCODE_2ND_L2R(i, k)] = twoArcScore(i, k, l);
				m_lSecondOrderScore[ENCODE_2ND_R2L(i, k)] = twoArcScore(l, k, i);
				//std::cout << i << " -> " << k << " with " << l << " : " << m_lSecondOrderScore[ENCODE_2ND_L2R(i, k)] << std::endl;
				//std::cout << l << " -> " << k << " with " << i << " : " << m_lSecondOrderScore[ENCODE_2ND_R2L(i, k)] << std::endl;
			}
		}
		int i = m_nSentenceLength - d + 1;
		m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] = twoArcScore(m_nSentenceLength, -1, i);
		//std::cout << m_nSentenceLength << " -> " << -1 << " with " << i << " : " << m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] << std::endl;
	}

	int DepParser::encodeEmptyWord(int i, int ec) {
		ttoken token =
			(i > 0 ? m_tWords.key(m_lSentence[i - 1][0].first()) : START_WORD) +
			m_tEmptys.key(ec) +
			(i < m_nSentenceLength ? m_tWords.key(m_lSentence[i][0].first()) : END_WORD);
		return m_tWords.lookup(token);
	}

	int DepParser::encodeEmptyPOSTag(int i, int ec) {
		ttoken token = "NN";
		return m_tPOSTags.lookup(token);
	}

	void DepParser::readEmptySentAndArcs(const DependencyTree & correct) {
		// for normal sentence
		m_nSentenceLength = 0;
		std::vector<int> tIds;
		std::vector<Arc> ecarcs;
		for (const auto & node : correct) {
			int h = TREENODE_HEAD(node), p = tIds.size();
			if (TREENODE_POSTAG(node) != EMPTYTAG) {
				tIds.push_back(m_nSentenceLength);
				m_lSentence[m_nSentenceLength++][0].refer(m_tWords.lookup(TREENODE_WORD(node)), m_tPOSTags.lookup(TREENODE_POSTAG(node)));
			}
			else {
				tIds.push_back(ENCODE_EMPTY(m_nSentenceLength, m_tEmptys.lookup(TREENODE_WORD(node))));
			}
			ecarcs.push_back(Arc(h, p));
			if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) return;
		}
		m_lSentence[m_nSentenceLength][0].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));
		// refer empty node
		for (int i = 0; i <= m_nSentenceLength; ++i) {
			for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
				m_lSentence[i][ec].refer(encodeEmptyWord(i, ec), encodeEmptyPOSTag(i, ec));
			}
		}
		for (const auto & ecarc : ecarcs) {
			m_vecCorrectArcs.push_back(Arc(ecarc.first() == -1 ? -1 : tIds[ecarc.first()], tIds[ecarc.second()]));
		}
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

	void DepParser::initScore(const DependencyTree & tree, float lambda) {
		m_fLambda = lambda;
		readEmptySentAndArcs(tree);

		for (int d = 1; d <= m_nSentenceLength + 1; ++d) {
			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
				int l = i + d - 1;
				if (d > 1) {
					m_lScore[i][l][0] = arcScore(i, i + d - 1);
					m_lScore[i][l][1] = arcScore(i + d - 1, i);
					//std::cout << i << ' ' << l << ' ' << m_lScore[i][l][0] << std::endl;
					//std::cout << l << ' ' << i << ' ' << m_lScore[i][l][1] << std::endl;
				}
				m_abScore[i][l][0].clear();
				m_abScore[i][l][1].clear();

				for (int t = 1; t <= MAX_EMPTY_SIZE; ++t) {
					if (m_nState == ParserState::GOLDTEST || m_pWeight->testEmptyNode(i, ENCODE_EMPTY(i + d, t), m_lSentence)) {
						const tscore & score = arcScore(i, ENCODE_EMPTY(i + d, t));
						m_abScore[i][l][0].insertItem(ScoreWithType(t, score));
					}
					if (m_nState == ParserState::GOLDTEST || m_pWeight->testEmptyNode(i + d - 1, ENCODE_EMPTY(i, t), m_lSentence)) {
						const tscore & score = arcScore(i + d - 1, ENCODE_EMPTY(i, t));
						m_abScore[i][l][1].insertItem(ScoreWithType(t, score));
					}
				}
				m_abScore[i][l][0].sortItems();
				m_abScore[i][l][1].sortItems();
			}
			int i = m_nSentenceLength - d + 1;
			m_lScore[i][m_nSentenceLength][1] = arcScore(m_nSentenceLength, i);
		}
	}

	void DepParser::decodeAfterScore() {
		for (int d = 1; d <= m_nSentenceLength + 1; ++d) {
			if (d > 1) {
				initSecondOrderScore(d);
			}
			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
				int l = i + d - 1;
				tscore l2r_arc_score = m_lScore[i][l][0];
				tscore r2l_arc_score = m_lScore[i][l][1];
				//std::cout << "()" << i << ' ' << l << ' ' << l2r_arc_score << std::endl;
				//std::cout << "()" << l << ' ' << i << ' ' << r2l_arc_score << std::endl;
				StateItem & item = m_lItems[d][i];

				// initialize
				item.init(i, l);

				for (int s = i; s < l; ++s) {
					const StateItem & litem = m_lItems[s - i + 1][i];
					const StateItem & ritem = m_lItems[l - s][s + 1];
					// jux
					item.updateJUX(s, litem.l2r.getScore() + ritem.r2l.getScore());
					// l2r_empty_inside
					tscore l_empty_inside_base_score = l2r_arc_score + ritem.r2l.getScore();
					for (const auto & l_beam : litem.l2r_empty_outside) {
						int p = ENCODE_EMPTY(s, DECODE_EMPTY_TAG(l_beam->getSplit()));
						int k = ENCODE_EMPTY(s + 1, DECODE_EMPTY_TAG(l_beam->getSplit()));
						int j = DECODE_EMPTY_POS(l_beam->getSplit());
						item.updateL2REmptyInside(p, l_beam->getSplit(), l_empty_inside_base_score +
							l_beam->getScore() +
							twoArcScore(i, k, l) +
							triArcScore(i, j == i ? -1 : j, k, l));
					}
					// r2l_empty_inside
					tscore r_empty_inside_base_score = r2l_arc_score + litem.l2r.getScore();
					for (const auto & r_beam : ritem.r2l_empty_outside) {
						int p = ENCODE_EMPTY(s, DECODE_EMPTY_TAG(r_beam->getSplit()));
						int k = ENCODE_EMPTY(s + 1, DECODE_EMPTY_TAG(r_beam->getSplit()));
						int j = DECODE_EMPTY_POS(r_beam->getSplit());
						item.updateR2LEmptyInside(p, r_beam->getSplit(), r_empty_inside_base_score +
							r_beam->getScore() +
							twoArcScore(l, k, i) +
							triArcScore(l, j == l ? -1 : j, k, i));
					}
				}
				for (int k = i + 1; k < l; ++k) {
					StateItem & litem = m_lItems[k - i + 1][i];
					StateItem & ritem = m_lItems[l - k + 1][k];

					const auto & l_beam = litem.l2r_solid_outside;
					const auto & r_beam = ritem.r2l_solid_outside;

					// l2r_solid_both
					tscore l_solid_both_base_score = ritem.jux.getScore() + l2r_arc_score + m_lSecondOrderScore[ENCODE_2ND_L2R(i, k)];
					for (const auto & swbs : l_beam) {
						// j is inner split
						const int & j = swbs->getSplit();
						item.updateL2RSolidBoth(k, j, l_solid_both_base_score +
							swbs->getScore() +
							triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, l));
					}
					// l2r_empty_outside
					for (const auto & swt : m_abScore[i][l][0]) {
						const int & t = swt->getType();
						// s is split with type
						int s = ENCODE_EMPTY(k, t);
						// o is outside empty
						int o = ENCODE_EMPTY(l + 1, t);
						tscore l_empty_outside_base_score = ritem.l2r.getScore() + swt->getScore() + twoArcScore(i, k, o);
						for (const auto & swbs : l_beam) {
							// j is inner split
							const int & j = swbs->getSplit();
							item.updateL2REmptyOutside(s, j, l_empty_outside_base_score +
								swbs->getScore() +
								triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o));
						}
					}
					// l2r
					item.updateL2R(k, litem.l2r_solid_outside.bestItem().getScore() + ritem.l2r.getScore());
					// r2l_solid_both
					tscore r_solid_both_base_score = litem.jux.getScore() + r2l_arc_score + m_lSecondOrderScore[ENCODE_2ND_R2L(i, k)];
					for (const auto & swbs : r_beam) {
						const int & j = swbs->getSplit();
						item.updateR2LSolidBoth(k, j, r_solid_both_base_score +
							swbs->getScore() +
							triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, i));
					}
					// r2l_empty_outside
					for (const auto & swt : m_abScore[i][l][1]) {
						const int & t = swt->getType();
						int s = ENCODE_EMPTY(k, t);
						int o = ENCODE_EMPTY(i, t);
						tscore r_empty_outside_base_score = litem.r2l.getScore() + swt->getScore() + twoArcScore(l, k, o);
						for (const auto & swbs : r_beam) {
							const int & j = swbs->getSplit();
							item.updateR2LEmptyOutside(s, j, r_empty_outside_base_score +
								swbs->getScore() +
								triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o));
						}
					}
					// r2l
					item.updateR2L(k, ritem.r2l_solid_outside.bestItem().getScore() + litem.r2l.getScore());
				}
				if (d > 1) {
					// l2r_solid_both
					item.updateL2RSolidBoth(i, i, m_lItems[d - 1][i + 1].r2l.getScore() +
						l2r_arc_score +
						m_lSecondOrderScore[ENCODE_2ND_L2R(i, i)] +
						triArcScore(i, -1, -1, l));
					// r2l_solid_both
					item.updateR2LSolidBoth(l, l, m_lItems[d - 1][i].l2r.getScore() +
						r2l_arc_score +
						m_lSecondOrderScore[ENCODE_2ND_R2L(i, i)] +
						triArcScore(l, -1, -1, i));
					// l2r_solid_outside
					for (const auto & swbs : item.l2r_empty_inside) {
						item.updateL2RSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
					for (const auto & swbs : item.l2r_solid_both) {
						item.updateL2RSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
					// r2l_solid_outside
					for (const auto & swbs : item.r2l_empty_inside) {
						item.updateR2LSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
					for (const auto & swbs : item.r2l_solid_both) {
						item.updateR2LSolidOutside(swbs->getSplit(), swbs->getInnerSplit(), swbs->getScore());
					}
				}
				// l2r_empty_ouside
				for (const auto & swt : m_abScore[i][l][0]) {
					const int & t = swt->getType();
					int s = ENCODE_EMPTY(l, t);
					int o = ENCODE_EMPTY(l + 1, t);
					if (d > 1) {
						tscore base_l2r_empty_outside_score = swt->getScore() + twoArcScore(i, l, o) + m_lItems[1][l].l2r.getScore();
						for (const auto & swbs : item.l2r_solid_outside) {
							const int & j = swbs->getSplit();
							item.updateL2REmptyOutside(s, swbs->getSplit(), base_l2r_empty_outside_score +
								swbs->getScore() +
								triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, l, o));
						}
					}
					else {
						item.updateL2REmptyOutside(s, i, swt->getScore() +
							twoArcScore(i, -1, o) +
							triArcScore(i, -1, -1, o));
					}
				}
				// r2l_empty_outside
				for (const auto & swt : m_abScore[i][l][1]) {
					const int & t = swt->getType();
					int s = ENCODE_EMPTY(i, t);
					int o = ENCODE_EMPTY(i, t);
					if (d > 1) {
						tscore base_r2l_empty_outside_score = swt->getScore() + twoArcScore(l, i, o) + m_lItems[1][i].r2l.getScore();
						for (const auto & swbs : item.r2l_solid_outside) {
							const int & j = swbs->getSplit();
							item.updateR2LEmptyOutside(s, swbs->getSplit(), base_r2l_empty_outside_score +
								swbs->getScore() +
								triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, i, o));
						}
					}
					else {
						item.updateR2LEmptyOutside(s, l, swt->getScore() +
							twoArcScore(l, -1, o) +
							triArcScore(l, -1, -1, o));
					}

				}
				// l2r
				item.updateL2R(l, item.l2r_solid_outside.bestItem().getScore() + m_lItems[1][l].l2r.getScore());
				if (item.l2r_empty_outside.size() > 0) {
					item.updateL2R(item.l2r_empty_outside.bestItem().getSplit(), item.l2r_empty_outside.bestItem().getScore());
				}
				// r2l
				item.updateR2L(i, item.r2l_solid_outside.bestItem().getScore() + m_lItems[1][i].r2l.getScore());
				if (item.r2l_empty_outside.size() > 0) {
					item.updateR2L(item.r2l_empty_outside.bestItem().getSplit(), item.r2l_empty_outside.bestItem().getScore());
				}

			}
			if (d > 1) {
				// root
				StateItem & item = m_lItems[d][m_nSentenceLength - d + 1];
				item.init(m_nSentenceLength - d + 1, m_nSentenceLength);
				// r2l_solid_outside
				item.updateR2LSolidOutside(m_nSentenceLength, m_nSentenceLength, m_lItems[d - 1][item.left].l2r.getScore() +
					m_lScore[item.left][item.right][1] +
					m_lSecondOrderScore[ENCODE_2ND_R2L(item.left, item.left)] +
					triArcScore(item.right, -1, -1, item.left));
				// r2l
				item.updateR2L(item.left, item.r2l_solid_outside.bestItem().getScore() + m_lItems[1][item.left].r2l.getScore());
				for (int i = item.left, s = item.left + 1, j = m_nSentenceLength + 1; s < j - 1; ++s) {
					item.updateR2L(s, m_lItems[j - s][s].r2l_solid_outside.bestItem().getScore() + m_lItems[s - i + 1][i].r2l.getScore());
				}
				//std::cout << "d = " << d << " score = " << item.r2l.getScore() << std::endl;
			}
		}
	}

	void DepParser::parseScore(const DependencyTree & tree, DependencyTree * retval) {
		DependencyTree correct;
		retval->clear();
		//std::cout << "here " << std::endl << tree << std::endl;
		for (const auto & node : tree) {
			if (TREENODE_POSTAG(node) != EMPTYTAG) {
				correct.push_back(node);
			}
		}
		decodeAfterScore();
		decodeArcs();
		generate(retval, correct);
	}
}
