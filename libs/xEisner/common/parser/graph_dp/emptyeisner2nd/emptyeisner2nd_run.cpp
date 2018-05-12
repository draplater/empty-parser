#include <ctime>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>

#include "emptyeisner2nd_run.h"
#include "emptyeisner2nd_depparser.h"

namespace emptyeisner2nd {
	Run::Run() = default;

	Run::~Run() = default;

	void Run::train(const std::string & sInputFile, const std::string & sFeatureInput, const std::string & sFeatureOutput) {
		int nRound = 0;
		DependencyTree ref_sent;

		std::cout << "Training iteration is started..." << std::endl;

		auto time_begin = time(NULL);

		std::unique_ptr<DepParser> parser(new DepParser(sFeatureInput, sFeatureOutput, ParserState::TRAIN));
		std::ifstream input(sInputFile);
		if (input) {
			while (input >> ref_sent) {
				for (const auto & token : ref_sent) {
					if (TREENODE_POSTAG(token) == EMPTYTAG) {
						parser->m_tEmptys.add(TREENODE_WORD(token));
					}
				}
			}
			std::cout << "empty tag complete" << std::endl << parser->m_tEmptys << std::endl;
			input.close();
			input.open(sInputFile);
		}
		if (input) {
			while (input >> ref_sent) {
				++nRound;
				parser->train(ref_sent, nRound);
			}
			parser->finishtraining();
		}
		input.close();

		auto time_end = time(NULL);

		std::cout << "Done." << std::endl;

		std::cout << "Training has finished successfully. Total time taken is: " << difftime(time_end, time_begin) << "s" << std::endl;
	}

	void Run::parse(const std::string & sInputFile, const std::string & sOutputFile, const std::string & sFeatureFile) {

		Sentence sentence;
		DependencyTree tree;

		std::cout << "Parsing started" << std::endl;

		auto time_begin = time(NULL);

		std::unique_ptr<DepParser> parser(new DepParser(sFeatureFile, sFeatureFile, ParserState::PARSE));
		std::ifstream input(sInputFile);
		std::ofstream output(sOutputFile);
		if (input) {
			while (input >> sentence) {
				tree.clear();
				parser->parse(sentence, &tree);
				if (!tree.empty()) {
					output << tree;
				}
			}
		}
		input.close();
		output.close();

		std::cout << "Parsing has finished successfully. Total time taken is: " << difftime(time(NULL), time_begin) << "s" << std::endl;
	}

	void Run::goldtest(const std::string & sInputFile, const std::string & sFeatureInput) {
		int nRound = 0;
		DependencyTree ref_sent;

		std::cout << "GoldTest iteration is started..." << std::endl;

		auto time_begin = time(NULL);

		std::unique_ptr<DepParser> parser(new DepParser(sFeatureInput, "", ParserState::GOLDTEST));
		std::ifstream input(sInputFile);
		if (input) {
			while (input >> ref_sent) {
				for (const auto & token : ref_sent) {
					if (TREENODE_POSTAG(token) == EMPTYTAG) {
						parser->m_tEmptys.add(TREENODE_WORD(token));
					}
				}
			}
			std::cout << "empty tag complete" << std::endl << parser->m_tEmptys << std::endl;
			input.close();
			input.open(sInputFile);
		}
		if (input) {
			while (input >> ref_sent) {
				++nRound;
				parser->train(ref_sent, nRound);
//				break;
			}
		}
		input.close();

		std::cout << "total " << parser->sentenceCount() << " round" << std::endl;

		auto time_end = time(NULL);

		std::cout << "Done." << std::endl;

		std::cout << "Training has finished successfully. Total time taken is: " << difftime(time_end, time_begin) << "s" << std::endl;
	}
}
