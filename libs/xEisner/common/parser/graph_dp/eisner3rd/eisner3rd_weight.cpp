#include <fstream>

#include "eisner3rd_weight.h"

namespace eisner3rd {
	Weight::Weight(const std::string & sRead, const std::string & sRecord, Token * words, Token * postags) :
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
		// third order
		m_mapPwC1pC2pC3p("m_mapPwC1pC2pC3p"),
		m_mapC1wPpC2pC3p("m_mapC1wPpC2pC3p"),
		m_mapC2wPpC1pC3p("m_mapC2wPpC1pC3p"),
		m_mapC3wPpC1pC2p("m_mapC3wPpC1pC2p"),
		m_mapPwC1wC2pC3p("m_mapPwC1wC2pC3p"),
		m_mapPwC2wC1pC3p("m_mapPwC2wC1pC3p"),
		m_mapPwC3wC1pC2p("m_mapPwC3wC1pC2p"),
		m_mapC1wC2wPpC3p("m_mapC1wC2wPpC3p"),
		m_mapC1wC3wPpC2p("m_mapC1wC3wPpC2p"),
		m_mapC2wC3wPpC1p("m_mapC2wC3wPpC1p"),
		m_mapPpC1pC2pC3p("m_mapPpC1pC2pC3p"),
		m_mapC1wC2pC3p("m_mapC1wC2pC3p"),
		m_mapC2wC1pC3p("m_mapC2wC1pC3p"),
		m_mapC3wC1pC2p("m_mapC3wC1pC2p"),
		m_mapC1wC2wC3p("m_mapC1wC2wC3p"),
		m_mapC1wC3wC2p("m_mapC1wC3wC2p"),
		m_mapC2wC3wC1p("m_mapC2wC3wC1p"),
		m_mapC1pC2pC3p("m_mapC1pC2pC3p"),
		m_mapC1wC3w("m_mapC1wC3w"),
		m_mapC1wC3p("m_mapC1wC3p"),
		m_mapC3wC1p("m_mapC3wC1p"),
		m_mapC1pC3p("m_mapC1pC3p")
	{
		m_pWords = words;
		m_pPOSTags = postags;
		loadScores();
		std::cout << "load complete." << std::endl;
	}

	Weight::~Weight() = default;

	void Weight::loadScores() {

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

		input >> m_mapPwC1pC2pC3p;
		input >> m_mapC1wPpC2pC3p;
		input >> m_mapC2wPpC1pC3p;
		input >> m_mapC3wPpC1pC2p;
		input >> m_mapPwC1wC2pC3p;
		input >> m_mapPwC2wC1pC3p;
		input >> m_mapPwC3wC1pC2p;
		input >> m_mapC1wC2wPpC3p;
		input >> m_mapC1wC3wPpC2p;
		input >> m_mapC2wC3wPpC1p;
		input >> m_mapPpC1pC2pC3p;
		input >> m_mapC1wC2pC3p;
		input >> m_mapC2wC1pC3p;
		input >> m_mapC3wC1pC2p;
		input >> m_mapC1wC2wC3p;
		input >> m_mapC1wC3wC2p;
		input >> m_mapC2wC3wC1p;
		input >> m_mapC1pC2pC3p;
		input >> m_mapC1wC3w;
		input >> m_mapC1wC3p;
		input >> m_mapC3wC1p;
		input >> m_mapC1pC3p;

		input.close();
	}

	void Weight::saveScores() const {

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

		output << m_mapPwC1pC2pC3p;
		output << m_mapC1wPpC2pC3p;
		output << m_mapC2wPpC1pC3p;
		output << m_mapC3wPpC1pC2p;
		output << m_mapPwC1wC2pC3p;
		output << m_mapPwC2wC1pC3p;
		output << m_mapPwC3wC1pC2p;
		output << m_mapC1wC2wPpC3p;
		output << m_mapC1wC3wPpC2p;
		output << m_mapC2wC3wPpC1p;
		output << m_mapPpC1pC2pC3p;
		output << m_mapC1wC2pC3p;
		output << m_mapC2wC1pC3p;
		output << m_mapC3wC1pC2p;
		output << m_mapC1wC2wC3p;
		output << m_mapC1wC3wC2p;
		output << m_mapC2wC3wC1p;
		output << m_mapC1pC2pC3p;
		output << m_mapC1wC3w;
		output << m_mapC1wC3p;
		output << m_mapC3wC1p;
		output << m_mapC1pC3p;

		output.close();
	}

	void Weight::computeAverageFeatureWeights(const int & round) {
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

		m_mapPwC1pC2pC3p.computeAverage(round);
		m_mapC1wPpC2pC3p.computeAverage(round);
		m_mapC2wPpC1pC3p.computeAverage(round);
		m_mapC3wPpC1pC2p.computeAverage(round);
		m_mapPwC1wC2pC3p.computeAverage(round);
		m_mapPwC2wC1pC3p.computeAverage(round);
		m_mapPwC3wC1pC2p.computeAverage(round);
		m_mapC1wC2wPpC3p.computeAverage(round);
		m_mapC1wC3wPpC2p.computeAverage(round);
		m_mapC2wC3wPpC1p.computeAverage(round);
		m_mapPpC1pC2pC3p.computeAverage(round);
		m_mapC1wC2pC3p.computeAverage(round);
		m_mapC2wC1pC3p.computeAverage(round);
		m_mapC3wC1pC2p.computeAverage(round);
		m_mapC1wC2wC3p.computeAverage(round);
		m_mapC1wC3wC2p.computeAverage(round);
		m_mapC2wC3wC1p.computeAverage(round);
		m_mapC1pC2pC3p.computeAverage(round);
		m_mapC1wC3w.computeAverage(round);
		m_mapC1wC3p.computeAverage(round);
		m_mapC3wC1p.computeAverage(round);
		m_mapC1pC3p.computeAverage(round);
	}
}