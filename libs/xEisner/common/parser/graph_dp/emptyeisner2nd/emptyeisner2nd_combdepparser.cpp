#include <cmath>
#include <stack>
#include <algorithm>
#include <unordered_set>

#include "emptyeisner2nd_combdepparser.h"

namespace emptyeisner2nd {

	WordPOSTag CombDepParser::empty_taggedword = WordPOSTag();
	WordPOSTag CombDepParser::start_taggedword = WordPOSTag();
	WordPOSTag CombDepParser::end_taggedword = WordPOSTag();

	CombDepParser::CombDepParser(const std::string & sFeatureInput, const std::string & sECFeatureInput, int nState) :
		DepParserBase(nState), m_tWords(1), m_tPOSTags(1), m_tECWords(1), m_tECPOSTags(1), m_tECEmptys(1), m_fLambda(-1.0) {

		m_nSentenceLength = 0;
		m_nSentenceCount = 0;

		m_pWeight = new Weight2nd(sFeatureInput, "", m_nScoreIndex, &m_tWords, &m_tPOSTags);
		m_pECWeight = new Weightec2nd(sECFeatureInput, "", m_nScoreIndex, &m_tECWords, &m_tECPOSTags, &m_tECEmptys);

		CombDepParser::empty_taggedword.refer(m_tECWords.lookup(EMPTY_WORD), m_tECPOSTags.lookup(EMPTY_POSTAG));
		CombDepParser::start_taggedword.refer(m_tECWords.lookup(START_WORD), m_tECPOSTags.lookup(START_POSTAG));
		CombDepParser::end_taggedword.refer(m_tECWords.lookup(END_WORD), m_tECPOSTags.lookup(END_POSTAG));

		m_pWeight->init(CombDepParser::empty_taggedword, CombDepParser::start_taggedword, CombDepParser::end_taggedword);
		m_pECWeight->init(CombDepParser::empty_taggedword, CombDepParser::start_taggedword, CombDepParser::end_taggedword);
	}

	CombDepParser::~CombDepParser() {
		delete m_pWeight;
	}

	int CombDepParser::sentenceCount() {
		return m_nSentenceCount;
	}

	int CombDepParser::encodeEmptyWord(int i, int ec) {
		ttoken token =
			(i > 0 ? m_tECWords.key(m_lSentence[i - 1][0].first()) : START_WORD) +
			m_tECEmptys.key(ec) +
			(i < m_nSentenceLength ? m_tECWords.key(m_lSentence[i][0].first()) : END_WORD);
		return m_tECWords.lookup(token);
	}

	int CombDepParser::encodeEmptyPOSTag(int i, int ec) {
		ttoken token = "NN";
		return m_tECPOSTags.lookup(token);
	}

	void CombDepParser::parse(const Sentence & sentence, DependencyTree * retval, float lambda) {
		int idx = 0;
		DependencyTree correct;
		m_fLambda = lambda;

		for (const auto & token : sentence) {
			if (SENT_POSTAG(token) != EMPTYTAG) {
				m_lOriginSentence[idx].refer(m_tWords.lookup(SENT_WORD(token)), m_tPOSTags.lookup(SENT_POSTAG(token)));
				m_lSentence[idx++][0].refer(m_tECWords.lookup(SENT_WORD(token)), m_tECPOSTags.lookup(SENT_POSTAG(token)));
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

		m_lOriginSentence[idx].refer(m_tWords.lookup(ROOT_WORD), m_tPOSTags.lookup(ROOT_POSTAG));
		m_lSentence[idx][0].refer(m_tECWords.lookup(ROOT_WORD), m_tECPOSTags.lookup(ROOT_POSTAG));		// refer empty node
		for (int i = 0; i <= idx; ++i) {
			for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
				m_lSentence[i][ec].refer(encodeEmptyWord(i, ec), encodeEmptyPOSTag(i, ec));
			}
		}
		work(retval, correct);
	}

	void CombDepParser::work(DependencyTree * retval, const DependencyTree & correct) {

		decode();

		switch (m_nState) {
		case ParserState::PARSE:
			decodeArcs();
			generate(retval, correct);
			break;
		default:
			break;
		}
	}

	void CombDepParser::initArcScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			int l = i, r = i + d - 1;
			if (d > 1) {
				m_lArcScore[l][r][0][0] = baseArcScore(l, r);
				m_lArcScore[l][r][1][0] = baseArcScore(r, l);
			}
			for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
				m_lArcScore[l][r][0][ec] = baseArcScore(l, ENCODE_EMPTY(r + 1, ec));
				m_lArcScore[l][r][1][ec] = baseArcScore(r, ENCODE_EMPTY(l, ec));
			}
		}
		if (d > 1) {
			int l = m_nSentenceLength - d + 1, r = m_nSentenceLength;
			m_lArcScore[l][r][1][0] = baseArcScore(r, l);
		}
	}

	void CombDepParser::initBiSiblingArcScore(const int & d) {
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

	void CombDepParser::decodeSpan(int distance, int left, int right) {
		for (int l = left; l < right; ++l) {

			int r = l + distance - 1;
			StateItem &item = m_lItems[l][r];
			const tscore(&l2r_arc_scores_list)[MAX_EMPTY_SIZE + 1] = m_lArcScore[l][r][0];
			const tscore(&r2l_arc_scores_list)[MAX_EMPTY_SIZE + 1] = m_lArcScore[l][r][1];
			const tscore(&l2r_bi_arc_scores_list)[MAX_SENTENCE_SIZE][MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = m_lBiSiblingScore[l][0];
			const tscore(&r2l_bi_arc_scores_list)[MAX_SENTENCE_SIZE][MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = m_lBiSiblingScore[l][1];

			// initialize
			item.init(l, r);

			for (int s = l; s < r; ++s) {

				const tscore(&l2r_bi_arc_scores)[MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = l2r_bi_arc_scores_list[s];
				const tscore(&r2l_bi_arc_scores)[MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = r2l_bi_arc_scores_list[s];

				StateItem &litem = m_lItems[l][s];
				StateItem &ritem = m_lItems[s + 1][r];

				// jux
				item.updateStates(
					litem.states[L2R].score + ritem.states[R2L].score,
					s, JUX);

				// l2r_empty_inside
				// split point would be encode as an empty point
				tscore l_empty_inside_base_score = l2r_arc_scores_list[0] + ritem.states[R2L].score;
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					item.updateStates(
						// bi-sibling arc score
						l2r_bi_arc_scores[ec][0] +
						// left part score
						litem.states[L2R_EMPTY_OUTSIDE + ec - 1].score +
						// arc score + right part score
						l_empty_inside_base_score,
						ENCODE_EMPTY(s, ec), L2R_EMPTY_INSIDE);
				}

				// r2l_empty_inside
				// split point would be encode as an empty point
				tscore r_empty_inside_base_score = r2l_arc_scores_list[0] + litem.states[L2R].score;
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					item.updateStates(
						// bi-sibling arc score
						r2l_bi_arc_scores[ec][0] +
						// right part score
						ritem.states[R2L_EMPTY_OUTSIDE + ec - 1].score +
						// arc score + left part score
						r_empty_inside_base_score,
						ENCODE_EMPTY(s, ec), R2L_EMPTY_INSIDE);
				}
			}

			for (int s = l + 1; s < r; ++s) {

				StateItem litem = m_lItems[l][s];
				StateItem ritem = m_lItems[s][r];

				const tscore(&l2r_bi_arc_scores)[MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = l2r_bi_arc_scores_list[s];
				const tscore(&r2l_bi_arc_scores)[MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = r2l_bi_arc_scores_list[s];


				// l2r_solid_both
				item.updateStates(
					// left part + right part
					litem.states[L2R_SOLID_OUTSIDE].score + ritem.states[JUX].score +
					// arc score
					l2r_arc_scores_list[0] +
					// bi-sibling arc score
					l2r_bi_arc_scores[0][0],
					s, L2R_SOLID_BOTH);
				// l2r
				item.updateStates(
					// left part + right part
					litem.states[L2R_SOLID_OUTSIDE].score + ritem.states[L2R].score,
					s, L2R);

				// r2l_solid_both
				item.updateStates(
					// left part + right part
					litem.states[JUX].score + ritem.states[R2L_SOLID_OUTSIDE].score +
					//arc score
					r2l_arc_scores_list[0] +
					// bi-sibling arc score
					r2l_bi_arc_scores[0][0],
					s, R2L_SOLID_BOTH);
				// r2l
				item.updateStates(
					// left part + right part
					litem.states[R2L].score + ritem.states[R2L_SOLID_OUTSIDE].score,
					s, R2L);

				tscore l_solid_base_score = litem.states[L2R_SOLID_OUTSIDE].score + ritem.states[L2R].score;
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					// l2r_empty_outside
					item.updateStates(
						// bi-sibling arc score
						l2r_bi_arc_scores[0][ec] +
						// arc score
						l2r_arc_scores_list[ec] +
						// left part + right part
						l_solid_base_score,
						s, L2R_EMPTY_OUTSIDE + ec - 1);
				}

				tscore r_solid_base_score = litem.states[R2L].score + ritem.states[R2L_SOLID_OUTSIDE].score;
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					// r2l_empty_outside
					item.updateStates(
						// bi-sibling arc score
						r2l_bi_arc_scores[0][ec] +
						// arc score
						r2l_arc_scores_list[ec] +
						// left part + right part
						r_solid_base_score,
						s, R2L_EMPTY_OUTSIDE + ec - 1);
				}
			}

			if (distance > 1) {
				StateItem &litem = m_lItems[l][r - 1];
				StateItem &ritem = m_lItems[l + 1][r];

				const tscore(&l2r_bi_arc_scores)[MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = l2r_bi_arc_scores_list[l];
				const tscore(&r2l_bi_arc_scores)[MAX_EMPTY_SIZE + 1][MAX_EMPTY_SIZE + 1] = r2l_bi_arc_scores_list[l];

				// l2r_solid_both
				item.updateStates(
					// right part score
					ritem.states[R2L].score +
					// arc score
					l2r_arc_scores_list[0] +
					// bi-sibling arc score
					l2r_bi_arc_scores[0][0],
					// left part is a point, 0 ec
					l, L2R_SOLID_BOTH);

				// r2l_solid_both
				item.updateStates(
					// left part score
					litem.states[L2R].score +
					// arc score
					r2l_arc_scores_list[0] +
					// bi-sibling arc score
					r2l_bi_arc_scores[0][0],
					// left part is L2R, lnec ec
					r, R2L_SOLID_BOTH);

				// l2r_solid_outside
				item.updateStates(
					item.states[L2R_SOLID_BOTH].score,
					item.states[L2R_SOLID_BOTH].split,
					L2R_SOLID_OUTSIDE);

				// l2r_solid_outside
				item.updateStates(
					item.states[L2R_EMPTY_INSIDE].score,
					item.states[L2R_EMPTY_INSIDE].split,
					L2R_SOLID_OUTSIDE);

				// r2l_solid_outside
				item.updateStates(
					item.states[R2L_SOLID_BOTH].score,
					item.states[R2L_SOLID_BOTH].split,
					R2L_SOLID_OUTSIDE);
				// r2l_solid_outside
				item.updateStates(
					item.states[R2L_EMPTY_INSIDE].score,
					item.states[R2L_EMPTY_INSIDE].split,
					R2L_SOLID_OUTSIDE);

				StateItem & ritem0 = m_lItems[r][r];
				StateItem & litem0 = item;
				// l2r_empty_ouside
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					item.updateStates(
						// left part + right part
						litem0.states[L2R_SOLID_OUTSIDE].score + ritem0.states[L2R].score +
						// arc score
						l2r_arc_scores_list[ec] +
						// bi-sibling arc score
						l2r_bi_arc_scores[0][ec],
						r, L2R_EMPTY_OUTSIDE + ec - 1);
				}
				// l2r
				item.updateStates(
					litem0.states[L2R_SOLID_OUTSIDE].score + ritem0.states[L2R].score,
					r, L2R);
				// l2r
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					item.updateStates(
						// empty outside
						item.states[L2R_EMPTY_OUTSIDE + ec - 1].score,
						ENCODE_EMPTY(r, ec), L2R);
				}

				StateItem & litem1 = m_lItems[l][l];
				StateItem & ritem1 = item;
				// r2l_empty_ouside
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					item.updateStates(
						// left part + right part
						ritem1.states[R2L_SOLID_OUTSIDE].score + litem1.states[R2L].score +
						// arc score
						r2l_arc_scores_list[ec] +
						// bi-sibling arc score
						r2l_bi_arc_scores[0][ec],
						l, R2L_EMPTY_OUTSIDE + ec - 1);
				}
				// r2l
				item.updateStates(
					ritem1.states[R2L_SOLID_OUTSIDE].score + litem1.states[R2L].score,
					l, R2L);
				// r2l
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					item.updateStates(
						// empty outside
						item.states[R2L_EMPTY_OUTSIDE + ec - 1].score,
						ENCODE_EMPTY(l, ec), R2L);
				}
			}
			else {
				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					// l2r_empty_ouside
					item.updateStates(
						l2r_bi_arc_scores_list[l][0][ec] +
						l2r_arc_scores_list[ec],
						r, L2R_EMPTY_OUTSIDE + ec - 1);
					// l2r with 1 empty node
					item.updateStates(
						item.states[L2R_EMPTY_OUTSIDE + ec - 1].score,
						ENCODE_EMPTY(l, ec), L2R);
				}
				// l2r
				item.updateStates(0, r, L2R);

				for (int ec = 1; ec <= MAX_EMPTY_SIZE; ++ec) {
					// r2l_empty_ouside
					item.updateStates(
						r2l_bi_arc_scores_list[l][0][ec] +
						r2l_arc_scores_list[ec],
						l, R2L_EMPTY_OUTSIDE + ec - 1);
					// r2l with 1 empty node
					item.updateStates(
						item.states[R2L_EMPTY_OUTSIDE + ec - 1].score,
						ENCODE_EMPTY(l, ec), R2L);
				}
				// r2l
				item.updateStates(0, l, R2L);
			}
			//item.print();
		}
	}

	void CombDepParser::decode(bool afterscore) {

		for (int d = 1; d <= m_nSentenceLength + 1; ++d) {
			if (!afterscore) {
				initArcScore(d);
			}
			initBiSiblingArcScore(d);
			decodeSpan(d, 0, m_nSentenceLength - d + 1);

			if (d > 1) {
				int l = m_nSentenceLength - d + 1, r = m_nSentenceLength;
				// root
				StateItem &l2ritem = m_lItems[l][r - 1];
				StateItem &item = m_lItems[l][r];

				// initialize
				item.init(l, r);
				// r2l_solid_outside
				item.updateStates(
					l2ritem.states[L2R].score +
					m_lBiSiblingScore[l][1][l][0][0] +
					m_lArcScore[l][r][1][0],
					r, R2L_SOLID_OUTSIDE);

				// r2l
				for (int s = l; s < r; ++s) {
					StateItem &litem = m_lItems[l][s];
					StateItem &ritem = m_lItems[s][r];
					item.updateStates(
						litem.states[R2L].score + ritem.states[R2L_SOLID_OUTSIDE].score,
						s, R2L);
				}

				//item.print();
			}
		}
	}

	void CombDepParser::decodeArcs() {

		m_vecTrainECArcs.clear();

		if (m_lItems[0][m_nSentenceLength].states[R2L].split == -1) return;
		typedef std::tuple<int, int, int> sItem;
		std::stack<sItem> stack;
		stack.push(sItem(0, m_nSentenceLength, R2L));

		while (!stack.empty()) {
			sItem span = stack.top();
			stack.pop();
			StateItem & item = m_lItems[std::get<0>(span)][std::get<1>(span)];
			item.type = std::get<2>(span);
			int split = item.states[item.type].split;

			//			std::cout << "total tecnum is " << tnec << std::endl;
			//			item.print(); //debug

			switch (item.type) {
			case JUX:
				stack.push(sItem(item.left, split, L2R));
				stack.push(sItem(split + 1, item.right, R2L));
				break;
			case L2R:
				if (IS_EMPTY(split)) {
					std::get<2>(span) = L2R_EMPTY_OUTSIDE + DECODE_EMPTY_TAG(split) - 1;
					stack.push(span);
					break;
				}
				if (item.left == item.right) {
					break;
				}

				stack.push(sItem(item.left, split, L2R_SOLID_OUTSIDE));
				stack.push(sItem(split, item.right, L2R));
				break;
			case R2L:
				if (IS_EMPTY(split)) {
					std::get<2>(span) = R2L_EMPTY_OUTSIDE + DECODE_EMPTY_TAG(split) - 1;
					stack.push(span);
					break;
				}
				if (item.left == item.right) {
					break;
				}

				stack.push(sItem(split, item.right, R2L_SOLID_OUTSIDE));
				stack.push(sItem(item.left, split, R2L));
				break;
			case L2R_SOLID_BOTH:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(ECArc(item.left, item.right));

				if (split == item.left) {
					stack.push(sItem(item.left + 1, item.right, R2L));
				}
				else {
					stack.push(sItem(item.left, split, L2R_SOLID_OUTSIDE));
					stack.push(sItem(split, item.right, JUX));
				}
				break;
			case R2L_SOLID_BOTH:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(ECArc(item.right == m_nSentenceLength ? -1 : item.right, item.left));

				if (split == item.right) {
					stack.push(sItem(item.left, item.right - 1, L2R));
				}
				else {
					stack.push(sItem(split, item.right, R2L_SOLID_OUTSIDE));
					stack.push(sItem(item.left, split, JUX));
				}
				break;
			case L2R_EMPTY_INSIDE:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(ECArc(item.left, item.right));

				stack.push(sItem(item.left, DECODE_EMPTY_POS(split), L2R_EMPTY_OUTSIDE + DECODE_EMPTY_TAG(split) - 1));
				stack.push(sItem(DECODE_EMPTY_POS(split) + 1, item.right, R2L));
				break;
			case R2L_EMPTY_INSIDE:
				if (item.left == item.right) {
					break;
				}
				m_vecTrainECArcs.push_back(ECArc(item.right, item.left));

				stack.push(sItem(DECODE_EMPTY_POS(split) + 1, item.right, R2L_EMPTY_OUTSIDE + DECODE_EMPTY_TAG(split) - 1));
				stack.push(sItem(item.left, DECODE_EMPTY_POS(split), L2R));
				break;
			case L2R_SOLID_OUTSIDE:
				if (item.left == item.right) {
					break;
				}

				std::get<2>(span) = IS_EMPTY(split) ? L2R_EMPTY_INSIDE : L2R_SOLID_BOTH;
				stack.push(span);
				break;
			case R2L_SOLID_OUTSIDE:
				if (item.left == item.right) {
					break;
				}
				if (item.right == m_nSentenceLength) {
					m_vecTrainECArcs.push_back(ECArc(-1, item.left));
					stack.push(sItem(item.left, item.right - 1, L2R));
					break;
				}

				std::get<2>(span) = IS_EMPTY(split) ? R2L_EMPTY_INSIDE : R2L_SOLID_BOTH;
				stack.push(span);
				break;
			case L2R_EMPTY_OUTSIDE + 0:
				m_vecTrainECArcs.push_back(ECArc(item.left, ENCODE_EMPTY(item.right + 1, item.type - L2R_EMPTY_OUTSIDE + 1)));

				if (item.left == item.right) {
					break;
				}

				stack.push(sItem(item.left, split, L2R_SOLID_OUTSIDE));
				stack.push(sItem(split, item.right, L2R));
				break;
			case R2L_EMPTY_OUTSIDE + 0:
				m_vecTrainECArcs.push_back(ECArc(item.right, ENCODE_EMPTY(item.left, item.type - R2L_EMPTY_OUTSIDE + 1)));

				if (item.left == item.right) {
					break;
				}

				stack.push(sItem(split, item.right, R2L_SOLID_OUTSIDE));
				stack.push(sItem(item.left, split, R2L));
				break;
			default:
				break;
			}
		}

	}

	void CombDepParser::generate(DependencyTree * retval, const DependencyTree & correct) {
		retval->clear();
		std::vector<ECArc> emptyArcs;
		for (const auto & arc : m_vecTrainECArcs) {
			if (IS_EMPTY(arc.second())) {
				emptyArcs.push_back(arc);
			}
		}
		std::sort(emptyArcs.begin(), emptyArcs.end(), [](const ECArc & arc1, const ECArc & arc2) { return compareArc(arc1, arc2); });

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
				retval->push_back(DependencyTreeNode(POSTaggedWord(m_tECEmptys.key(DECODE_EMPTY_TAG((itr_e++)->second())), EMPTYTAG), -1, NULL_LABEL));
			}
		}
		while (idx != m_nSentenceLength) {
			nidmap[idx] = nidx++;
			retval->push_back(DependencyTreeNode(TREENODE_POSTAGGEDWORD(correct[idx++]), -1, NULL_LABEL));
		}
		while (itr_e != emptyArcs.end()) {
			nidmap[(itr_e->second() << MAX_SENTENCE_BITS) + itr_e->first()] = nidx++;
			retval->push_back(DependencyTreeNode(POSTaggedWord(m_tECEmptys.key(DECODE_EMPTY_TAG((itr_e++)->second())), EMPTYTAG), -1, NULL_LABEL));
		}

		for (const auto & arc : m_vecTrainECArcs) {
			TREENODE_HEAD(retval->at(nidmap[IS_EMPTY(arc.second()) ? ((arc.second() << MAX_SENTENCE_BITS) + arc.first()) : arc.second()])) = nidmap[arc.first()];
		}
	}

	tscore CombDepParser::baseArcScore(const int & p, const int & c) {
		return getOrUpdateBaseArcScore(p, c, 0);
	}

	tscore CombDepParser::biSiblingArcScore(const int & p, const int & c, const int & c2) {
		return getOrUpdateBiSiblingScore(p, c, c2, 0);
	}

	tscore CombDepParser::getOrUpdateBaseArcScore(const int & p, const int & c, const int & amount) {
		tscore score1 = 0;
		if (!IS_EMPTY(c)) {
			score1 = m_pWeight->getOrUpdateArcScore(p, c, amount, m_nSentenceLength, m_lOriginSentence);
		}
		tscore score2 = m_pECWeight->getOrUpdateBaseArcScore(p, c, amount, m_nSentenceLength, m_lSentence);
		if (m_fLambda > 0) return m_fLambda * score1 + (1.0 - m_fLambda) * score2;
		else return score1 + score2;
	}

	tscore CombDepParser::getOrUpdateBiSiblingScore(const int & p, const int & c, const int & c2, const int & amount) {
		tscore score1 = 0;
		if (!IS_EMPTY(c) && !IS_EMPTY(c2)) {
			score1 = m_pWeight->getOrUpdateBiArcScore(p, c, c2, amount, m_nSentenceLength, m_lOriginSentence);
		}
		tscore score2 = m_pECWeight->getOrUpdateBiArcScore(p, c, c2, amount, m_nSentenceLength, m_lSentence);
		if (m_fLambda > 0) return m_fLambda * score1 + (1.0 - m_fLambda) * score2;
		else return score1 + score2;
	}
}
