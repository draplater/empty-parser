#include <fstream>

#include "weight2nd.h"

Weight2nd::Weight2nd(const std::string & sRead, const std::string & sRecord, int nParserState, Token * words, Token * postags) :
	Weight1st(sRead, sRecord, nParserState, words, postags),
	// second order
	m_mapC1pC2p("m_mapC1pC2p"),
	m_mapPpC1pC2p("m_mapPpC1pC2p"),
	m_mapC1wC2w("m_mapC1wC2w"),
	m_mapC1wC2p("m_mapC1wC2p"),
	m_mapC2wC1p("m_mapC2wC1p")
{
	loadTokens(words, postags);
	loadScores();
	// std::cout << "load complete." << std::endl;
}

Weight2nd::~Weight2nd() = default;

void Weight2nd::loadTokens(Token * words, Token * postags) {
	m_pWords = words;
	m_pPOSTags = postags;
}

void Weight2nd::loadScores() {

	if (m_sReadPath.empty()) {
		return;
	}
	std::ifstream input(m_sReadPath);
	if (!input) {
		return;
	}

	input >> *m_pWords;

	input >> *m_pPOSTags;

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

void Weight2nd::saveScores() const {

	if (m_sRecordPath.empty()) {
		return;
	}
	std::ofstream output(m_sRecordPath);
	if (!output) {
		return;
	}

	output << *m_pWords;

	output << *m_pPOSTags;

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

void Weight2nd::computeAverageFeatureWeights(const int & round) {
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

tscore Weight2nd::getOrUpdateBiArcScore(const int & p, const int & c, const int & c2, const int & amount, int sentLen, WordPOSTag (&sent)[MAX_SENTENCE_SIZE]) {
	tscore retval = 0;
	// elements
	int dir = encodeLinkDistanceOrDirection(p, c2, true);

	Word c_word(IS_NULL(c) ? m_tkEmpty.first() : sent[c].first()), c2_word(sent[c2].first());
	POSTag p_tag(sent[p].second()), c_tag(IS_NULL(c) ? m_tkEmpty.second() : sent[c].second()), c2_tag(sent[c2].second());

	// features

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
