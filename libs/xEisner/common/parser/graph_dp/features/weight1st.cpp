#include <fstream>

#include "weight1st.h"

Weight1st::Weight1st(const std::string & sRead, const std::string & sRecord, int nParserState, Token * words, Token * postags) :
	WeightBase(sRead, sRecord),
	m_mapPw("m_mapPw"),
	m_mapPp("m_mapPp"),
	m_mapPwp("m_mapPwp"),
	m_mapCw("m_mapCw"),
	m_mapCp("m_mapCp"),
	m_mapCwp("m_mapCwp"),
	m_mapPwpCwp("m_mapPwpCwp"),
	m_mapPpCwp("m_mapPpCwp"),
	m_mapPwpCp("m_mapPwpCp"),
	m_mapPwCwp("m_mapPwCwp"),
	m_mapPwpCw("m_mapPwpCw"),
	m_mapPwCw("m_mapPwCw"),
	m_mapPpCp("m_mapPpCp"),
	m_mapPpBpCp("m_mapPpBpCp"),
	m_mapPpPp1Cp_1Cp("m_mapPpPp1Cp_1Cp"),
	m_mapPp_1PpCp_1Cp("m_mapPp_1PpCp_1Cp"),
	m_mapPpPp1CpCp1("m_mapPpPp1CpCp1"),
	m_mapPp_1PpCpCp1("m_mapPp_1PpCpCp1"),
	m_nScoreIndex(nParserState),
	m_nTrainingRound(0)
{
	m_pUnusedWords = words;
	m_pUnusedPOSTags = postags;
	loadScores();
	// std::cout << "load complete." << std::endl;
}

Weight1st::~Weight1st() = default;

void Weight1st::loadScores() {

	if (m_sReadPath.empty()) {
		return;
	}
	std::ifstream input(m_sReadPath);
	if (!input) {
		return;
	}

	input >> *m_pUnusedWords;

	input >> *m_pUnusedPOSTags;

	input >> m_mapPw;
	input >> m_mapPp;
	input >> m_mapPwp;

	input >> m_mapCw;
	input >> m_mapCp;
	input >> m_mapCwp;

	input >> m_mapPwpCwp;
	input >> m_mapPpCwp;
	input >> m_mapPwpCp;
	input >> m_mapPwCwp;
	input >> m_mapPwpCw;
	input >> m_mapPwCw;
	input >> m_mapPpCp;

	input >> m_mapPpBpCp;
	input >> m_mapPpPp1Cp_1Cp;
	input >> m_mapPp_1PpCp_1Cp;
	input >> m_mapPpPp1CpCp1;
	input >> m_mapPp_1PpCpCp1;

	input.close();
}

void Weight1st::saveScores() const {

	if (m_sRecordPath.empty()) {
		return;
	}
	std::ofstream output(m_sRecordPath);
	if (!output) {
		return;
	}

	output << *m_pUnusedWords;

	output << *m_pUnusedPOSTags;

	output << m_mapPw;
	output << m_mapPp;
	output << m_mapPwp;

	output << m_mapCw;
	output << m_mapCp;
	output << m_mapCwp;

	output << m_mapPwpCwp;
	output << m_mapPpCwp;
	output << m_mapPwpCp;
	output << m_mapPwCwp;
	output << m_mapPwpCw;
	output << m_mapPwCw;
	output << m_mapPpCp;

	output << m_mapPpBpCp;
	output << m_mapPpPp1Cp_1Cp;
	output << m_mapPp_1PpCp_1Cp;
	output << m_mapPpPp1CpCp1;
	output << m_mapPp_1PpCpCp1;

	output.close();
}

void Weight1st::computeAverageFeatureWeights(const int & round) {
	m_mapPw.computeAverage(round);
	m_mapPp.computeAverage(round);
	m_mapPwp.computeAverage(round);

	m_mapCw.computeAverage(round);
	m_mapCp.computeAverage(round);
	m_mapCwp.computeAverage(round);
	m_mapPwpCwp.computeAverage(round);
	m_mapPpCwp.computeAverage(round);
	m_mapPwpCp.computeAverage(round);
	m_mapPwCwp.computeAverage(round);
	m_mapPwpCw.computeAverage(round);
	m_mapPwCw.computeAverage(round);
	m_mapPpCp.computeAverage(round);
	m_mapPpBpCp.computeAverage(round);
	m_mapPpPp1Cp_1Cp.computeAverage(round);
	m_mapPp_1PpCp_1Cp.computeAverage(round);
	m_mapPpPp1CpCp1.computeAverage(round);
	m_mapPp_1PpCpCp1.computeAverage(round);
}

void Weight1st::init(const WordPOSTag & tkEmpty, const WordPOSTag & tkStart, const WordPOSTag & tkEnd) {
	m_tkEmpty.refer(tkEmpty.first(), tkEmpty.second());
	m_tkStart.refer(tkStart.first(), tkStart.second());
	m_tkEnd.refer(tkEnd.first(), tkEnd.second());
}

void Weight1st::referRound(const int & nRound) { m_nTrainingRound = nRound; }

tscore Weight1st::getOrUpdateArcScore(const int & p, const int & c, const int & amount, int sentLen, WordPOSTag (&sent)[MAX_SENTENCE_SIZE]) {

	tscore retval = 0;

	// elements
	int dir = encodeLinkDistanceOrDirection(p, c, true);
	int dis = encodeLinkDistanceOrDirection(p, c, false);

	Word p_word(sent[p].first()), c_word(sent[c].first());
	POSTag p_tag(sent[p].second()), c_tag(sent[c].second());

	int pp = p - 1, np = p + 1, pc = c - 1, nc = c + 1;

	POSTag p_1_tag(pp >= 0 ? sent[pp].second() : m_tkStart.second()),
			p1_tag(np < sentLen ? sent[np].second() : m_tkEnd.second()),
			c_1_tag(pc >= 0 ? sent[pc].second() : m_tkStart.second()),
			c1_tag(nc < sentLen ? sent[nc].second() : m_tkEnd.second()),
			b_tag;

	// features
	WordInt word_int;
	POSTagInt tag_int;

	TwoWordsInt word_word_int;
	POSTagSet2Int tag_tag_int;
	WordPOSTagInt word_tag_int;

	POSTagSet3Int tag_tag_tag_int;
	WordPOSTagPOSTagInt word_tag_tag_int;
	WordWordPOSTagInt word_word_tag_int;

	POSTagSet4Int tag_tag_tag_tag_int;
	WordWordPOSTagPOSTagInt word_word_tag_tag_int;

	word_int.refer(p_word, 0);
	m_mapPw.getOrUpdateScore(retval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_int.referLast(dis);
	m_mapPw.getOrUpdateScore(retval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_int.referLast(dir);
	m_mapPw.getOrUpdateScore(retval, word_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_int.refer(p_tag, 0);
	m_mapPp.getOrUpdateScore(retval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_int.referLast(dis);
	m_mapPp.getOrUpdateScore(retval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_int.referLast(dir);
	m_mapPp.getOrUpdateScore(retval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_tag_int.refer(p_word, p_tag, 0);
	m_mapPwp.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_int.referLast(dis);
	m_mapPwp.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_int.referLast(dir);
	m_mapPwp.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_int.refer(c_word, 0);
	m_mapCw.getOrUpdateScore(retval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_int.referLast(dis);
	m_mapCw.getOrUpdateScore(retval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_int.referLast(dir);
	m_mapCw.getOrUpdateScore(retval, word_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_int.refer(c_tag, 0);
	m_mapCp.getOrUpdateScore(retval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_int.referLast(dis);
	m_mapCp.getOrUpdateScore(retval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_int.referLast(dir);
	m_mapCp.getOrUpdateScore(retval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_tag_int.refer(c_word, c_tag, 0);
	m_mapCwp.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_int.referLast(dis);
	m_mapCwp.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_int.referLast(dir);
	m_mapCwp.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_word_tag_tag_int.refer(p_word, c_word, p_tag, c_tag, 0);
	m_mapPwpCwp.getOrUpdateScore(retval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_tag_tag_int.referLast(dis);
	m_mapPwpCwp.getOrUpdateScore(retval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_tag_tag_int.referLast(dir);
	m_mapPwpCwp.getOrUpdateScore(retval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_tag_tag_int.refer(p_word, p_tag, c_tag, 0);
	m_mapPwpCp.getOrUpdateScore(retval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_tag_int.referLast(dis);
	m_mapPwpCp.getOrUpdateScore(retval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_tag_int.referLast(dir);
	m_mapPwpCp.getOrUpdateScore(retval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_tag_tag_int.refer(c_word, p_tag, c_tag, 0);
	m_mapPpCwp.getOrUpdateScore(retval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_tag_int.referLast(dis);
	m_mapPpCwp.getOrUpdateScore(retval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_tag_int.referLast(dir);
	m_mapPpCwp.getOrUpdateScore(retval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_word_tag_int.refer(p_word, c_word, p_tag, 0);
	m_mapPwpCw.getOrUpdateScore(retval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_tag_int.referLast(dis);
	m_mapPwpCw.getOrUpdateScore(retval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_tag_int.referLast(dir);
	m_mapPwpCw.getOrUpdateScore(retval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_word_tag_int.refer(p_word, c_word, c_tag, 0);
	m_mapPwCwp.getOrUpdateScore(retval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_tag_int.referLast(dis);
	m_mapPwCwp.getOrUpdateScore(retval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_tag_int.referLast(dir);
	m_mapPwCwp.getOrUpdateScore(retval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_word_int.refer(p_word, c_word, 0);
	m_mapPwCw.getOrUpdateScore(retval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_int.referLast(dis);
	m_mapPwCw.getOrUpdateScore(retval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_int.referLast(dir);
	m_mapPwCw.getOrUpdateScore(retval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_int.refer(p_tag, c_tag, 0);
	m_mapPpCp.getOrUpdateScore(retval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_int.referLast(dis);
	m_mapPpCp.getOrUpdateScore(retval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_int.referLast(dir);
	m_mapPpCp.getOrUpdateScore(retval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_1_tag, c_tag, 0);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_1_tag, c_tag, 0);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_tag, c1_tag, 0);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_tag, c1_tag, 0);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(m_tkEmpty.second(), p1_tag, c_1_tag, c_tag, 0);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.refer(m_tkEmpty.second(), p1_tag, c_1_tag, c_tag, dir);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_1_tag, m_tkEmpty.second(), c_1_tag, c_tag, 0);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(m_tkEmpty.second(), p1_tag, c_tag, c1_tag, 0);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_1_tag, m_tkEmpty.second(), c_tag, c1_tag, 0);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_1_tag, m_tkEmpty.second(), 0);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_1_tag, m_tkEmpty.second(), 0);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_tag, p1_tag, m_tkEmpty.second(), c1_tag, 0);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_1_tag, p_tag, m_tkEmpty.second(), c1_tag, 0);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_tag, m_tkEmpty.second(), c_1_tag, c_tag, 0);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPpPp1Cp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(m_tkEmpty.second(), p_tag, c_1_tag, c_tag, 0);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCp_1Cp.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_tag, m_tkEmpty.second(), 0);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPpPp1CpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_tag, m_tkEmpty.second(), 0);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dis);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_tag_int.referLast(dir);
	m_mapPp_1PpCpCp1.getOrUpdateScore(retval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	for (int b = (int)std::fmin(p, c) + 1, e = (int)std::fmax(p, c); b < e; ++b) {
		b_tag = sent[b].second();
		tag_tag_tag_int.refer(p_tag, b_tag, c_tag, 0);
		m_mapPpBpCp.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.refer(p_tag, b_tag, c_tag, dis);
		m_mapPpBpCp.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.refer(p_tag, b_tag, c_tag, dir);
		m_mapPpBpCp.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	}

	return retval;
}
