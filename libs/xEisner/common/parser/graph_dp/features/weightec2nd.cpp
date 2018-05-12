#include <fstream>

#include "weightec2nd.h"

Weightec2nd::Weightec2nd(const std::string & sRead, const std::string & sRecord, int nParserState, Token * words, Token * postags, Token * emptys) :
	WeightBase(sRead, sRecord),
	// first order
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
	// second order
	m_mapC1pC2p("m_mapC1pC2p"),
	m_mapPpC1pC2p("m_mapPpC1pC2p"),
	m_mapC1wC2w("m_mapC1wC2w"),
	m_mapC1wC2p("m_mapC1wC2p"),
	m_mapC2wC1p("m_mapC2wC1p"),
	m_nScoreIndex(nParserState),
	m_nTrainingRound(0)
{
	loadTokens(words, postags, emptys);
	loadScores();
//	std::cout << "load complete." << std::endl;
}

Weightec2nd::~Weightec2nd() = default;

void Weightec2nd::loadTokens(Token * words, Token * postags, Token * emptys) {
	m_pWords = words;
	m_pPOSTags = postags;
	m_pEmptys = emptys;
}

void Weightec2nd::loadScores() {

	if (m_sReadPath.empty()) {
		return;
	}
	std::ifstream input(m_sReadPath);
	if (!input) {
		return;
	}

	input >> *m_pWords;

	input >> *m_pPOSTags;

	input >> *m_pEmptys;

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

	input >> m_mapC1pC2p;
	input >> m_mapPpC1pC2p;
	input >> m_mapC1wC2w;
	input >> m_mapC1wC2p;
	input >> m_mapC2wC1p;

	input.close();
}

void Weightec2nd::saveScores() const {

	if (m_sRecordPath.empty()) {
		return;
	}
	std::ofstream output(m_sRecordPath);
	if (!output) {
		return;
	}

	output << *m_pWords;

	output << *m_pPOSTags;

	output << *m_pEmptys;

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

	output << m_mapC1pC2p;
	output << m_mapPpC1pC2p;
	output << m_mapC1wC2w;
	output << m_mapC1wC2p;
	output << m_mapC2wC1p;

	output.close();
}

void Weightec2nd::computeAverageFeatureWeights(const int & round) {
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

	m_mapC1pC2p.computeAverage(round);
	m_mapPpC1pC2p.computeAverage(round);
	m_mapC1wC2w.computeAverage(round);
	m_mapC1wC2p.computeAverage(round);
	m_mapC2wC1p.computeAverage(round);
}

void Weightec2nd::init(const WordPOSTag & tkEmpty, const WordPOSTag & tkStart, const WordPOSTag & tkEnd) {
	m_tkEmpty.refer(tkEmpty.first(), tkEmpty.second());
	m_tkStart.refer(tkStart.first(), tkStart.second());
	m_tkEnd.refer(tkEnd.first(), tkEnd.second());
}

void Weightec2nd::referRound(const int & nRound) { m_nTrainingRound = nRound; }

tscore Weightec2nd::getOrUpdateBaseArcScore(const int & p, const int & c, const int & amount, int sentLen, WordPOSTag sent[MAX_SENTENCE_SIZE][MAX_EMPTYTAG_SIZE]) {

	tscore retval = 0;

	// elements
	int pos_c = DECODE_EMPTY_POS(c);
	int tag_c = DECODE_EMPTY_TAG(c);
	int pc = IS_EMPTY(c) ? (pos_c > p ? pos_c : pos_c - 1) : c;

	int dis = tag_c == 0 ? encodeLinkDistanceOrDirection(p, pc, false) : encodeEmptyDistance(p, pc);
	int dir = encodeLinkDistanceOrDirection(p, pc, true);

//	if (dis == 0 || dir == 0) std::cout << "p = " << p << " c = " << c << " fuck you" << std::endl;

	Word p_word(sent[p][0].first()), c_word(sent[pos_c][tag_c].first());
	POSTag p_tag(sent[p][0].second()), c_tag(sent[pos_c][tag_c].second());
	POSTag p_1_tag(p > 0 ? sent[p - 1][0].second() : m_tkStart.second()),
			p1_tag(p < sentLen - 1 ? sent[p + 1][0].second() : m_tkEnd.second()),
			c_1_tag(pos_c > 0 ? sent[pos_c - 1][0].second() : m_tkStart.second()), c1_tag, b_tag;
	if (tag_c == 0) {
		c1_tag = pos_c < sentLen - 1 ? sent[pos_c + 1][0].second() : m_tkEnd.second();
	}
	else {
		c1_tag = pos_c < sentLen ? sent[pos_c][0].second() : m_tkEnd.second();
	}

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

	if (tag_c == 0) {

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
	}

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

	// inner normal
	for (int b = (pos_c <= p ? (tag_c == 0 ? pos_c + 1 : pos_c) : (p + 1)), e = std::max(p, pos_c); b < e; ++b) {
		b_tag = sent[b][0].second();
		tag_tag_tag_int.refer(p_tag, b_tag, c_tag, 0);
		m_mapPpBpCp.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.refer(p_tag, b_tag, c_tag, dis);
		m_mapPpBpCp.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.refer(p_tag, b_tag, c_tag, dir);
		m_mapPpBpCp.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	}

	return retval;
}

tscore Weightec2nd::getOrUpdateBiArcScore(const int & p, const int & c, const int & c2, const int & amount, int sentLen, WordPOSTag (&sent)[MAX_SENTENCE_SIZE][MAX_EMPTYTAG_SIZE]) {

	tscore retval = 0;

	int pos_c = DECODE_EMPTY_POS(c);
	int tag_c = DECODE_EMPTY_TAG(c);

	int pos_c2 = DECODE_EMPTY_POS(c2);
	int tag_c2 = DECODE_EMPTY_TAG(c2);

	int p_tag = sent[p][0].second();

	int pc = IS_EMPTY(c2) ? (pos_c2 > p ? pos_c2 : pos_c2 - 1) : c2;
	int dir = encodeLinkDistanceOrDirection(p, pc, true);

	Word c_word = IS_NULL(c) ? m_tkEmpty.first() : sent[pos_c][tag_c].first();
	POSTag c_tag = IS_NULL(c) ? m_tkEmpty.second() : sent[pos_c][tag_c].second();

	Word c2_word = sent[pos_c2][tag_c2].first();
	POSTag c2_tag = sent[pos_c2][tag_c2].second();

	TwoWordsInt word_word_int;
	POSTagSet2Int tag_tag_int;
	WordPOSTagInt word_tag_int;
	POSTagSet3Int tag_tag_tag_int;

	tag_tag_int.refer(c_tag, c2_tag, 0);
	m_mapC1pC2p.getOrUpdateScore(retval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_int.referLast(dir);
	m_mapC1pC2p.getOrUpdateScore(retval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	tag_tag_tag_int.refer(p_tag, c_tag, c2_tag, 0);
	m_mapPpC1pC2p.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	tag_tag_tag_int.referLast(dir);
	m_mapPpC1pC2p.getOrUpdateScore(retval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_word_int.refer(c_word, c2_word, 0);
	m_mapC1wC2w.getOrUpdateScore(retval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_word_int.referLast(dir);
	m_mapC1wC2w.getOrUpdateScore(retval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_tag_int.refer(c_word, c2_tag, 0);
	m_mapC1wC2p.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_int.referLast(dir);
	m_mapC1wC2p.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	word_tag_int.refer(c2_word, c_tag, 0);
	m_mapC2wC1p.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	word_tag_int.referLast(dir);
	m_mapC2wC1p.getOrUpdateScore(retval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	return retval;
}
