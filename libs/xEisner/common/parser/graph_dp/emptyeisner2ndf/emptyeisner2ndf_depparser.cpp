#include <cmath>
#include <stack>
#include <algorithm>
#include <unordered_set>

#include "emptyeisner2ndf_depparser.h"

namespace emptyeisner2ndf {

	WordPOSTag DepParser::empty_taggedword = WordPOSTag();
	WordPOSTag DepParser::start_taggedword = WordPOSTag();
	WordPOSTag DepParser::end_taggedword = WordPOSTag();

	DepParser::DepParser(const std::string & sFeatureInput, const std::string & sFeatureOut, int nState) :
		DepParserBase(nState), m_tWords(1), m_tPOSTags(1), m_tEmptys(1), m_fLambda(-1.0) {

		m_nSentenceLength = 0;
		m_nSentenceCount = 0;

		m_pWeight = new Weightec2nd(sFeatureInput, sFeatureOut, m_nScoreIndex, &m_tWords, &m_tPOSTags, &m_tEmptys);

		DepParser::empty_taggedword.refer(m_tWords.lookup(EMPTY_WORD), m_tPOSTags.lookup(EMPTY_POSTAG));
		DepParser::start_taggedword.refer(m_tWords.lookup(START_WORD), m_tPOSTags.lookup(START_POSTAG));
		DepParser::end_taggedword.refer(m_tWords.lookup(END_WORD), m_tPOSTags.lookup(END_POSTAG));

		m_pWeight->init(DepParser::empty_taggedword, DepParser::start_taggedword, DepParser::end_taggedword);
	}

	DepParser::~DepParser() {
		delete m_pWeight;
	}

	int DepParser::sentenceCount() {
		return m_nSentenceCount;
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
		std::vector<ECArc> ecarcs;
		for (const auto & node : correct) {
			int h = TREENODE_HEAD(node), p = tIds.size();
			if (TREENODE_POSTAG(node) != EMPTYTAG) {
				tIds.push_back(m_nSentenceLength);
				m_lSentence[m_nSentenceLength++][0].refer(m_tWords.lookup(TREENODE_WORD(node)), m_tPOSTags.lookup(TREENODE_POSTAG(node)));
			}
			else {
				tIds.push_back(ENCODE_EMPTY(m_nSentenceLength, m_tEmptys.lookup(TREENODE_WORD(node))));
			}
			ecarcs.push_back(ECArc(h, p));
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
			m_vecCorrectECArcs.push_back(ECArc(ecarc.first() == -1 ? -1 : tIds[ecarc.first()], tIds[ecarc.second()]));
		}
	}

	void DepParser::train(const DependencyTree & correct, const int & round) {
		// initialize
		m_vecCorrectECArcs.clear();
		m_nSentenceLength = 0;

		readEmptySentAndArcs(correct);

		if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) {
			std::cout << "skip one" << std::endl;
			return;
		}

		++m_nSentenceCount;

		Arcs2BiArcs(m_vecCorrectECArcs, m_vecCorrectBiArcs);

		if (m_nState == ParserState::GOLDTEST) {
			m_setArcGoldScore.clear();
			m_setBiSiblingArcGoldScore.clear();
			for (const auto & ecarc : m_vecCorrectECArcs) {
				m_setArcGoldScore.insert(ECArc(ecarc.first() == -1 ? m_nSentenceLength : ecarc.first(), ecarc.second()));
//				std::cout << "from " << ecarc.first() << " to " << ecarc.second() << " with " << ecarc.third() << " empty" << std::endl;
			}
			for (const auto & biarc : m_vecCorrectBiArcs) {
				m_setBiSiblingArcGoldScore.insert(ECBiArc(biarc.first(), biarc.second(), biarc.third()));
			}
		}

		m_pWeight->referRound(m_nSentenceCount);
		work(nullptr, correct);
		if (m_nSentenceCount % OUTPUT_STEP == 0) {
			std::cout << m_nTotalErrors << " / " << m_nTrainingRound << " with " << m_nSentenceCount << " sent" << std::endl;
		}
	}

	void DepParser::parse(const Sentence & sentence, DependencyTree * retval) {
		int idx = 0;
		DependencyTree correct;

		for (const auto & token : sentence) {
			if (SENT_POSTAG(token) != EMPTYTAG) {
				m_lSentence[idx++][0].refer(m_tWords.lookup(SENT_WORD(token)), m_tPOSTags.lookup(SENT_POSTAG(token)));
				correct.push_back(DependencyTreeNode(token, -1, NULL_LABEL));
				if (idx == MAX_SENTENCE_SIZE - 1) {
					std::cout << "skip one" << std::endl;
					return;
				}
			}
		}
		++m_nTrainingRound;
		m_nSentenceLength = correct.size();

		if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) {
			std::cout << "skip one" << std::endl;
			return;
		}

		m_lSentence[idx][0].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));		// refer empty node
		for (int i = 0; i <= idx; ++i) {
			for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
				m_lSentence[i][ec].refer(encodeEmptyWord(i, ec), encodeEmptyPOSTag(i, ec));
			}
		}
		work(retval, correct);
	}

	void DepParser::parseScore(const DependencyTree & tree, DependencyTree * retval) {
		DependencyTree correct;
		for (const auto & node : tree) {
			if (TREENODE_POSTAG(node) != EMPTYTAG) {
				correct.push_back(node);
			}
		}

		decode(true);

		//std::cout << "real empty is " << m_nRealEmpty << std::endl;

		decodeArcs();
		generate(retval, correct);
	}

	void DepParser::work(DependencyTree * retval, const DependencyTree & correct) {

		decode();

		switch (m_nState) {
		case ParserState::TRAIN:
			decodeArcs();
			update();
			break;
		case ParserState::PARSE:
			decodeArcs();
			generate(retval, correct);
			break;
		case ParserState::GOLDTEST:
			++m_nTrainingRound;
			decodeArcs();
			goldCheck();
			break;
		default:
			break;
		}
	}

	void DepParser::initScore(const DependencyTree & tree, float lambda) {
		m_fLambda = lambda;
		// for normal sentence
		m_nSentenceLength = 0;
		std::vector<int> tIds;
		for (const auto & node : tree) {
			int h = TREENODE_HEAD(node), p = tIds.size();
			if (TREENODE_POSTAG(node) != EMPTYTAG) {
				tIds.push_back(m_nSentenceLength);
				m_lSentence[m_nSentenceLength++][0].refer(m_tWords.lookup(TREENODE_WORD(node)), m_tPOSTags.lookup(TREENODE_POSTAG(node)));
			}
			else {
				tIds.push_back(ENCODE_EMPTY(m_nSentenceLength, m_tEmptys.lookup(TREENODE_WORD(node))));
			}
			if (m_nSentenceLength >= MAX_SENTENCE_SIZE - 1) return;
		}
		m_lSentence[m_nSentenceLength][0].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));
		// refer empty node
		for (int i = 0; i <= m_nSentenceLength; ++i) {
			for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
				m_lSentence[i][ec].refer(encodeEmptyWord(i, ec), encodeEmptyPOSTag(i, ec));
			}
		}

		for (int d = 1; d <= m_nSentenceLength + 1; ++d) {
			initArcScore(d);
		}
	}

	void DepParser::initArcScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			int l = i, r = i + d - 1;
			if (d > 1) {
				m_lArcScore[l][r][0][0] = baseArcScore(l, r);
				m_lArcScore[l][r][1][0] = baseArcScore(r, l);
				//std::cout << l << " -> " << r << " : " << m_lArcScore[l][r][0][0] << std::endl;
				//std::cout << r << " -> " << l << " : " << m_lArcScore[l][r][1][0] << std::endl;
			}
			for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
				m_lArcScore[l][r][0][ec] = baseArcScore(l, ENCODE_EMPTY(r + 1, ec));
				m_lArcScore[l][r][1][ec] = baseArcScore(r, ENCODE_EMPTY(l, ec));
				//std::cout << l << " -> " << r << " with empty : " << m_lArcScore[l][r][0][0] << std::endl;
				//std::cout << r << " -> " << l << " with empty : " << m_lArcScore[l][r][1][0] << std::endl;
			}
		}
		if (d > 1) {
			int l = m_nSentenceLength - d + 1, r = m_nSentenceLength;
			m_lArcScore[l][r][1][0] = baseArcScore(r, l);
			//std::cout << r << " -> " << l << " : " << m_lArcScore[l][r][1][0] << std::endl;
		}
	}

	void DepParser::initBiSiblingArcScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			int l = i + d - 1;
			if (d > 1) {
				m_lBiSiblingScore[i][0][i][0][0] = biSiblingArcScore(i, -1, l);
				m_lBiSiblingScore[i][1][i][0][0] = biSiblingArcScore(l, -1, i);
			}
			if (d == 1) {
				for (int out_ec = 1; out_ec <= MAX_EMPTY_SIZE; ++out_ec) {
					m_lBiSiblingScore[i][0][i][0][out_ec] = biSiblingArcScore(i, -1, ENCODE_EMPTY(l + 1, out_ec));
					m_lBiSiblingScore[i][1][i][0][out_ec] = biSiblingArcScore(l, -1, ENCODE_EMPTY(i, out_ec));
				}
			}
			else {
				for (int out_ec = 1; out_ec <= MAX_EMPTY_SIZE; ++out_ec) {
					m_lBiSiblingScore[i][0][i][0][out_ec] = biSiblingArcScore(i, l, ENCODE_EMPTY(l + 1, out_ec));
					m_lBiSiblingScore[i][1][i][0][out_ec] = biSiblingArcScore(l, i, ENCODE_EMPTY(i, out_ec));
				}
			}
			for (int mid_ec = 1; mid_ec <= MAX_EMPTY_SIZE; ++mid_ec) {
				m_lBiSiblingScore[i][0][i][mid_ec][0] = biSiblingArcScore(i, ENCODE_EMPTY(i + 1, mid_ec), l);
				m_lBiSiblingScore[i][1][i][mid_ec][0] = biSiblingArcScore(l, ENCODE_EMPTY(i + 1, mid_ec), i);
				//if (i == 30 && l == 31) std::cout << "l2r inside score = " << m_lBiSiblingScore[i][0][i][mid_ec][0] << std::endl;
			}
			for (int k = i + 1, l = i + d - 1; k < l; ++k) {
				if (d > 1) {
					m_lBiSiblingScore[i][0][k][0][0] = biSiblingArcScore(i, k, l);
					m_lBiSiblingScore[i][1][k][0][0] = biSiblingArcScore(l, k, i);
				}
				for (int mid_ec = 1; mid_ec <= MAX_EMPTY_SIZE; ++mid_ec) {
					m_lBiSiblingScore[i][0][k][mid_ec][0] = biSiblingArcScore(i, ENCODE_EMPTY(k + 1, mid_ec), l);
					m_lBiSiblingScore[i][1][k][mid_ec][0] = biSiblingArcScore(l, ENCODE_EMPTY(k + 1, mid_ec), i);
				}
				for (int out_ec = 1; out_ec <= MAX_EMPTY_SIZE; ++out_ec) {
					m_lBiSiblingScore[i][0][k][0][out_ec] = biSiblingArcScore(i, k, ENCODE_EMPTY(l + 1, out_ec));
					m_lBiSiblingScore[i][1][k][0][out_ec] = biSiblingArcScore(l, k, ENCODE_EMPTY(i, out_ec));
				}
			}
		}
		m_lBiSiblingScore[m_nSentenceLength - d + 1][1][m_nSentenceLength - d + 1][0][0] = biSiblingArcScore(m_nSentenceLength, -1, m_nSentenceLength - d + 1);
	}


	void DepParser::decode(bool afterscore) {
		for (int d = 1; d <= m_nSentenceLength + 1; ++d) {
			if (!afterscore) initArcScore(d);
			initBiSiblingArcScore(d);
			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
				int l = i + d - 1;
				const tscore(&l2r_arc_scores_list)[MAX_EMPTY_SIZE + 1] = m_lArcScore[i][l][0];
				const tscore(&r2l_arc_scores_list)[MAX_EMPTY_SIZE + 1] = m_lArcScore[i][l][1];
				const tscore(&l2r_bi_arc_scores_list)[MAX_SENTENCE_SIZE][MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = m_lBiSiblingScore[i][0];
				const tscore(&r2l_bi_arc_scores_list)[MAX_SENTENCE_SIZE][MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = m_lBiSiblingScore[i][1];
				StateItem & item = m_lItems[i][l];

				// initialize
				item.init(i, l);

				for (int s = i; s < l; ++s) {
					const StateItem & litem = m_lItems[i][s];
					const StateItem & ritem = m_lItems[s + 1][l];
					// jux
					item.updateJUX(s, litem.l2r.getScore() + ritem.r2l.getScore());
					// l2r_empty_inside
					tscore l_empty_inside_base_score = l2r_arc_scores_list[0] + ritem.r2l.getScore();
					for (const auto & l_beam : litem.l2r_empty_outside) {
						int t = DECODE_EMPTY_TAG(l_beam->getSplit());
						int p = ENCODE_EMPTY(s, t);
						int k = ENCODE_EMPTY(s + 1, t);
						int j = DECODE_EMPTY_POS(l_beam->getSplit());
						item.updateL2REmptyInside(p, l_beam->getSplit(), l_empty_inside_base_score +
							l_beam->getScore() +
							l2r_bi_arc_scores_list[s][t][0] +
							biSiblingArcScore(i, j == i ? -1 : j, l));
						//if (i == 35 && l == 40) {
							//std::cout << "(1)" << std::endl;
							//std::cout << i << " -> " << k << " with " << l << " : " << l2r_bi_arc_scores_list[s][t][0] << std::endl;
							//std::cout << i << " -> " << (j == i ? -1 : j) << " with " << l << " : " << biSiblingArcScore(i, j == i ? -1 : j, l) << std::endl;
						//}
					}
					// r2l_empty_inside
					tscore r_empty_inside_base_score = r2l_arc_scores_list[0] + litem.l2r.getScore();
					for (const auto & r_beam : ritem.r2l_empty_outside) {
						int t = DECODE_EMPTY_TAG(r_beam->getSplit());
						int p = ENCODE_EMPTY(s, t);
						int k = ENCODE_EMPTY(s + 1, t);
						int j = DECODE_EMPTY_POS(r_beam->getSplit());
						item.updateR2LEmptyInside(p, r_beam->getSplit(), r_empty_inside_base_score +
							r_beam->getScore() +
							r2l_bi_arc_scores_list[s][t][0] +
							biSiblingArcScore(l, j == l ? -1 : j, i));
						//if (i == 3 && l == 4) {
							//std::cout << "(2)" << std::endl;
							//std::cout << l << " -> " << k << " with " << i << " : " << r2l_bi_arc_scores_list[s][t][0] << std::endl;
							//std::cout << l << " -> " << (j == l ? -1 : j) << " with " << i << " : " << biSiblingArcScore(l, j == l ? -1 : j, i) << std::endl;
						//}
					}
				}
				for (int k = i + 1; k < l; ++k) {
					StateItem & litem = m_lItems[i][k];
					StateItem & ritem = m_lItems[k][l];

					const auto & l_beam = litem.l2r_solid_outside;
					const auto & r_beam = ritem.r2l_solid_outside;

					// l2r_solid_both
					tscore l_solid_both_base_score = ritem.jux.getScore() + l2r_arc_scores_list[0] + l2r_bi_arc_scores_list[k][0][0];
					// j is inner split
					item.updateL2RSolidBoth(k, l_beam.getSplit(), l_solid_both_base_score +
						l_beam.getScore());
					//std::cout << "(3)" << std::endl;
					//std::cout << i << " -> " << (j == i ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << l << " : " << triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, l) << std::endl;
					// l2r_empty_outside
					for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
						const int & t = ec;
						// j is inner split
						int j = l_beam.getSplit();
						// s is split with type
						int s = ENCODE_EMPTY(k, t);
						// o is outside empty
						int o = ENCODE_EMPTY(l + 1, t);
						tscore l_empty_outside_base_score = ritem.l2r.getScore() + l2r_arc_scores_list[t] + l2r_bi_arc_scores_list[k][0][t];
						// j is inner split
						item.updateL2REmptyOutside(s, j, l_empty_outside_base_score +
							l_beam.getScore());
						//std::cout << "(4)" << std::endl;
						//std::cout << i << " -> " << (j == i ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << o << " : " << triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o) << std::endl;
					}
					// l2r
					item.updateL2R(k, litem.l2r_solid_outside.getScore() + ritem.l2r.getScore());
					// r2l_solid_both
					tscore r_solid_both_base_score = litem.jux.getScore() + r2l_arc_scores_list[0] + r2l_bi_arc_scores_list[k][0][0];
					item.updateR2LSolidBoth(k, r_beam.getSplit(), r_solid_both_base_score +
						r_beam.getScore());
					//std::cout << "(5)" << std::endl;
					//std::cout << l << " -> " << (j == l ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << i << " : " << triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, i) << std::endl;
					// r2l_empty_outside
					for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
						const int & t = ec;
						int j = r_beam.getSplit();
						int s = ENCODE_EMPTY(k, t);
						int o = ENCODE_EMPTY(i, t);
						tscore r_empty_outside_base_score = litem.r2l.getScore() + r2l_arc_scores_list[t] + r2l_bi_arc_scores_list[k][0][t];
						// j is inner split
						item.updateR2LEmptyOutside(s, j, r_empty_outside_base_score +
							r_beam.getScore());
						//std::cout << "(6)" << std::endl;
						//std::cout << l << " -> " << (j == l ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << k << " with " << o << " : " << triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, k, o) << std::endl;
					}
					// r2l
					item.updateR2L(k, ritem.r2l_solid_outside.getScore() + litem.r2l.getScore());
				}
				if (d > 1) {
					// l2r_solid_both
					item.updateL2RSolidBoth(i, i, m_lItems[i + 1][l].r2l.getScore() +
						l2r_arc_scores_list[0] +
						l2r_bi_arc_scores_list[i][0][0]);
					//std::cout << "(7)" << std::endl;
					//std::cout << i << " -> " << -1 << " with " << -1 << " with " << l << " : " << triArcScore(i, -1, -1, l) << std::endl;
					// r2l_solid_both
					item.updateR2LSolidBoth(l, l, m_lItems[i][l - 1].l2r.getScore() +
						r2l_arc_scores_list[0] +
						r2l_bi_arc_scores_list[i][0][0]);
					//std::cout << "(8)" << std::endl;
					//std::cout << l << " -> " << -1 << " with " << -1 << " with " << i << " : " << triArcScore(l, -1, -1, i) << std::endl;
					// l2r_solid_outside
					item.updateL2RSolidOutside(item.l2r_empty_inside.getSplit(), item.l2r_empty_inside.getInnerSplit(), item.l2r_empty_inside.getScore());
					item.updateL2RSolidOutside(item.l2r_solid_both.getSplit(), item.l2r_solid_both.getInnerSplit(), item.l2r_solid_both.getScore());
					// r2l_solid_outside
					item.updateR2LSolidOutside(item.r2l_empty_inside.getSplit(), item.r2l_empty_inside.getInnerSplit(), item.r2l_empty_inside.getScore());
					item.updateR2LSolidOutside(item.r2l_solid_both.getSplit(), item.r2l_solid_both.getInnerSplit(), item.r2l_solid_both.getScore());
				}
				// l2r_empty_ouside
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					const int & t = ec;
					int s = ENCODE_EMPTY(l, t);
					int o = ENCODE_EMPTY(l + 1, t);
					if (d > 1) {
						tscore base_l2r_empty_outside_score = l2r_arc_scores_list[t] + l2r_bi_arc_scores_list[i][0][t] + m_lItems[l][l].l2r.getScore();
						const int & j = item.l2r_solid_outside.getSplit();
						//std::cout << "score " << swbs->getScore() << std::endl;
						item.updateL2REmptyOutside(s, j, base_l2r_empty_outside_score +
							item.l2r_solid_outside.getScore());
						//std::cout << "(9)" << std::endl;
						//std::cout << i << " -> " << (j == i ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << l << " with " << o << " : " << triArcScore(i, j == i ? -1 : IS_EMPTY(j) ? j + 1 : j, l, o) << std::endl;
					}
					else {
						item.updateL2REmptyOutside(s, i, l2r_arc_scores_list[t] +
							l2r_bi_arc_scores_list[i][0][t]);
						//std::cout << "(10)" << std::endl;
						//std::cout << i << " -> " << -1 << " with " << o << twoArcScore(i, -1, o) << std::endl;
						//std::cout << i << " -> " << -1 << " with " << -1 << " with " << o << " : " << triArcScore(i, -1, -1, o) << std::endl;
					}
				}
				// r2l_empty_outside
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					const int & t = ec;
					int s = ENCODE_EMPTY(i, t);
					int o = ENCODE_EMPTY(i, t);
					if (d > 1) {
						tscore base_r2l_empty_outside_score = r2l_arc_scores_list[t] + r2l_bi_arc_scores_list[i][0][t] + m_lItems[i][i].r2l.getScore();
						const int & j = item.r2l_solid_outside.getSplit();
						item.updateR2LEmptyOutside(s, j, base_r2l_empty_outside_score +
							item.r2l_solid_outside.getScore());
						//std::cout << "(11)" << std::endl;
						//std::cout << l << " -> " << (j == l ? -1 : IS_EMPTY(j) ? j + 1 : j) << " with " << i << " with " << o << " : " << triArcScore(l, j == l ? -1 : IS_EMPTY(j) ? j + 1 : j, i, o) << std::endl;
					}
					else {
						item.updateR2LEmptyOutside(s, l, r2l_arc_scores_list[t] +
							r2l_bi_arc_scores_list[i][0][t]);
						//std::cout << "(12)" << std::endl;
						//std::cout << l << " -> " << -1 << " with " << i << twoArcScore(l, -1, o) << std::endl;
						//std::cout << l << " -> " << -1 << " with " << -1 << " with " << o << " : " << triArcScore(l, -1, -1, o) << std::endl;
					}

				}
				// l2r
				item.updateL2R(l, item.l2r_solid_outside.getScore() + m_lItems[l][l].l2r.getScore());
				if (item.l2r_empty_outside.size() > 0) {
					item.updateL2R(item.l2r_empty_outside.bestItem().getSplit(), item.l2r_empty_outside.bestItem().getScore());
				}
				// r2l
				item.updateR2L(i, item.r2l_solid_outside.getScore() + m_lItems[i][i].r2l.getScore());
				if (item.r2l_empty_outside.size() > 0) {
					item.updateR2L(item.r2l_empty_outside.bestItem().getSplit(), item.r2l_empty_outside.bestItem().getScore());
				}
			}
			if (d > 1) {
				// root
				StateItem & item = m_lItems[m_nSentenceLength - d + 1][m_nSentenceLength];
				item.init(m_nSentenceLength - d + 1, m_nSentenceLength);
				// r2l_solid_outside
				item.updateR2LSolidOutside(m_nSentenceLength, m_nSentenceLength, m_lItems[item.left][item.right - 1].l2r.getScore() +
					m_lArcScore[item.left][item.right][1][0] +
					m_lBiSiblingScore[item.left][1][item.left][0][0]);
				//std::cout << "(13)" << std::endl;
				//std::cout << item.right << " -> -1 with -1 with " << item.left << " : " << triArcScore(item.right, -1, -1, item.left) << std::endl;
				// r2l
				item.updateR2L(item.left, item.r2l_solid_outside.getScore() + m_lItems[item.left][item.left].r2l.getScore());
				for (int i = item.left, s = item.left + 1, j = m_nSentenceLength + 1; s < j - 1; ++s) {
					item.updateR2L(s, m_lItems[s][j - 1].r2l_solid_outside.getScore() + m_lItems[i][s].r2l.getScore());
				}
			}
		}
	}

	void DepParser::decodeArcs() {
		m_vecTrainECArcs.clear();
		std::stack<std::tuple<int, int, int, int>> stack;
		stack.push(std::tuple<int, int, int, int>(m_nSentenceLength + 1, -1, 0, R2L));
		m_lItems[0][m_nSentenceLength].type = R2L;

		while (!stack.empty()) {
			std::tuple<int, int, int, int> span = stack.top();
			stack.pop();
			StateItem & item = m_lItems[std::get<2>(span)][std::get<0>(span) + std::get<2>(span) - 1];
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

				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, m_lItems[item.left][split].l2r_solid_outside.getSplit(), item.left, L2R_SOLID_OUTSIDE));
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

				stack.push(std::tuple<int, int, int, int>(item.right - split + 1, m_lItems[split][item.right].r2l_solid_outside.getSplit(), split, R2L_SOLID_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, R2L));
				break;
			case L2R_SOLID_BOTH:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(BiGram<int>(item.left, item.right));

				if (split == item.left) {
					stack.push(std::tuple<int, int, int, int>(item.right - split, -1, split + 1, R2L));
				}
				else {
					innersplit = item.l2r_solid_both.getInnerSplit();

					stack.push(std::tuple<int, int, int, int>(split - item.left + 1, innersplit, item.left, L2R_SOLID_OUTSIDE));
					stack.push(std::tuple<int, int, int, int>(item.right - split + 1, -1, split, JUX));
				}
				break;
			case R2L_SOLID_BOTH:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(BiGram<int>(item.right == m_nSentenceLength ? -1 : item.right, item.left));

				if (split == item.right) {
					stack.push(std::tuple<int, int, int, int>(split - item.left, -1, item.left, L2R));
				}
				else {
					innersplit = item.r2l_solid_both.getInnerSplit();

					stack.push(std::tuple<int, int, int, int>(item.right - split + 1, innersplit, split, R2L_SOLID_OUTSIDE));
					stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, JUX));
				}
				break;
			case L2R_EMPTY_INSIDE:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(BiGram<int>(item.left, item.right));

				innersplit = item.l2r_empty_inside.getInnerSplit();
				split = DECODE_EMPTY_POS(split);

				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, innersplit, item.left, L2R_EMPTY_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(item.right - split, -1, split + 1, R2L));
				break;
			case R2L_EMPTY_INSIDE:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(BiGram<int>(item.right, item.left));

				innersplit = item.r2l_empty_inside.getInnerSplit();
				split = DECODE_EMPTY_POS(split);

				stack.push(std::tuple<int, int, int, int>(item.right - split, innersplit, split + 1, R2L_EMPTY_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, -1, item.left, L2R));
				break;
			case L2R_EMPTY_OUTSIDE:
				m_vecTrainECArcs.push_back(BiGram<int>(item.left, ENCODE_EMPTY(item.right + 1, DECODE_EMPTY_TAG(split))));

				if (item.left == item.right) {
					break;
				}

				innersplit = findInnerSplit(item.l2r_empty_outside, split);
				split = DECODE_EMPTY_POS(split);

				stack.push(std::tuple<int, int, int, int>(split - item.left + 1, innersplit, item.left, L2R_SOLID_OUTSIDE));
				stack.push(std::tuple<int, int, int, int>(item.right - split + 1, -1, split, L2R));
				break;
			case R2L_EMPTY_OUTSIDE:
				m_vecTrainECArcs.push_back(BiGram<int>(item.right, ENCODE_EMPTY(item.left, DECODE_EMPTY_TAG(split))));

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
					m_vecTrainECArcs.push_back(BiGram<int>(-1, item.left));
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

	}

	void DepParser::update() {
		Arcs2BiArcs(m_vecTrainECArcs, m_vecTrainBiArcs);

		std::unordered_set<ECArc> positiveArcs;
		positiveArcs.insert(m_vecCorrectECArcs.begin(), m_vecCorrectECArcs.end());
		for (const auto & arc : m_vecTrainECArcs) {
			positiveArcs.erase(arc);
		}
		std::unordered_set<ECArc> negativeArcs;
		negativeArcs.insert(m_vecTrainECArcs.begin(), m_vecTrainECArcs.end());
		for (const auto & arc : m_vecCorrectECArcs) {
			negativeArcs.erase(arc);
		}
		for (const auto & arc : positiveArcs) {
			getOrUpdateBaseArcScore(arc.first(), arc.second(), 1);
		}
		for (const auto & arc : negativeArcs) {
			getOrUpdateBaseArcScore(arc.first(), arc.second(), -1);
		}

		std::unordered_set<ECBiArc> positiveBiArcs;
		positiveBiArcs.insert(m_vecCorrectBiArcs.begin(), m_vecCorrectBiArcs.end());
		for (const auto & arc : m_vecTrainBiArcs) {
			positiveBiArcs.erase(arc);
		}
		std::unordered_set<ECBiArc> negativeBiArcs;
		negativeBiArcs.insert(m_vecTrainBiArcs.begin(), m_vecTrainBiArcs.end());
		for (const auto & arc : m_vecCorrectBiArcs) {
			negativeBiArcs.erase(arc);
		}
		if (!positiveBiArcs.empty() || !negativeBiArcs.empty()) {
			++m_nTotalErrors;
		}
		++m_nTrainingRound;
		for (const auto & arc : positiveBiArcs) {
			getOrUpdateBiSiblingScore(arc.first(), arc.second(), arc.third(), 1);
		}
		for (const auto & arc : negativeBiArcs) {
			getOrUpdateBiSiblingScore(arc.first(), arc.second(), arc.third(), -1);
		}
	}

	void DepParser::generate(DependencyTree * retval, const DependencyTree & correct) {
		retval->clear();
		std::vector<ECArc> emptyArcs;
		for (const auto & arc : m_vecTrainECArcs) {
			if (IS_EMPTY(arc.second())) {
				emptyArcs.push_back(arc);
			}
		}
		std::sort(emptyArcs.begin(), emptyArcs.end(), [](const ECArc & arc1, const ECArc & arc2){ return compareArc(arc1, arc2); });

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

		for (const auto & arc : m_vecTrainECArcs) {
			TREENODE_HEAD(retval->at(nidmap[IS_EMPTY(arc.second()) ? ((arc.second() << MAX_SENTENCE_BITS) + arc.first()) : arc.second()])) = nidmap[arc.first()];
		}
	}

	void DepParser::goldCheck() {
		Arcs2BiArcs(m_vecTrainECArcs, m_vecTrainBiArcs);
		//std::cout << "correct arcs" << std::endl;
		//for (const auto & arc : m_vecCorrectECArcs) {
		//	std::cout << arc.first() << " -> " << arc.second() << std::endl;
		//}
		//std::cout << "correct biarcs" << std::endl;
		//for (const auto & biarc : m_vecCorrectBiArcs) {
		//	std::cout << biarc.first() << " -> " << biarc.second() << " , " << biarc.third() << std::endl;
		//}
		//std::cout << "train arcs" << std::endl;
		//for (const auto & arc : m_vecTrainECArcs) {
		//	std::cout << arc.first() << " -> " << arc.second() << std::endl;
		//}
		//std::cout << "train biarcs" << std::endl;
		//for (const auto & biarc : m_vecTrainBiArcs) {
		//	std::cout << biarc.first() << " -> " << biarc.second() << " , " << biarc.third() << std::endl;
		//}
		if (m_vecCorrectECArcs.size() != m_vecTrainECArcs.size() || m_lItems[0][m_nSentenceLength].r2l.getScore() / GOLD_POS_SCORE != m_vecCorrectECArcs.size() + m_vecCorrectBiArcs.size()) {
			std::cout << "gold parse len error at " << m_nTrainingRound << std::endl;
			std::cout << "score is " << m_lItems[0][m_nSentenceLength].r2l.getScore() << std::endl;
			std::cout << "len is " << m_vecTrainECArcs.size() << std::endl;
			++m_nTotalErrors;
		}
		else {
			int i = 0;
			std::sort(m_vecCorrectECArcs.begin(), m_vecCorrectECArcs.end(), [](const ECArc & arc1, const ECArc & arc2){ return arc1 < arc2; });
			std::sort(m_vecTrainECArcs.begin(), m_vecTrainECArcs.end(), [](const ECArc & arc1, const ECArc & arc2){ return arc1 < arc2; });
			for (int n = m_vecCorrectECArcs.size(); i < n; ++i) {
				if (m_vecCorrectECArcs[i].first() != m_vecTrainECArcs[i].first() || m_vecCorrectECArcs[i].second() != m_vecTrainECArcs[i].second()) {
					break;
				}
			}
			if (i != m_vecCorrectECArcs.size()) {
				std::cout << "gold parse tree error at " << m_nTrainingRound << std::endl;
				for (const auto & biarc : m_vecTrainBiArcs) {
					std::cout << biarc << std::endl;
				}
				++m_nTotalErrors;
			}
		}
	}

	tscore DepParser::baseArcScore(const int & p, const int & c) {
		if (m_nState == ParserState::GOLDTEST) {
			return m_setArcGoldScore.find(ECArc(p, c)) == m_setArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
		}
		return getOrUpdateBaseArcScore(p, c, 0);
	}

	tscore DepParser::biSiblingArcScore(const int & p, const int & c, const int & c2) {
		if (m_nState == ParserState::GOLDTEST) {
			return m_setBiSiblingArcGoldScore.find(ECBiArc(p, c, c2)) == m_setBiSiblingArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
		}
		return getOrUpdateBiSiblingScore(p, c, c2, 0);
	}

	tscore DepParser::getOrUpdateBaseArcScore(const int & p, const int & c, const int & amount) {
		tscore retval = m_pWeight->getOrUpdateBaseArcScore(p, c, amount, m_nSentenceLength, m_lSentence);
		if (m_fLambda > 0) retval *= m_fLambda;
		return retval;
	}

	tscore DepParser::getOrUpdateBiSiblingScore(const int & p, const int & c, const int & c2, const int & amount) {
		tscore retval = m_pWeight->getOrUpdateBiArcScore(p, c, c2, amount, m_nSentenceLength, m_lSentence);
		if (m_fLambda > 0) retval *= m_fLambda;
		return retval;
	}
}
