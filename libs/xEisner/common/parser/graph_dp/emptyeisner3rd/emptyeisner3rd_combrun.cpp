#include <ctime>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>

#include "emptyeisner3rd_combrun.h"
#include "emptyeisner3rd_combdepparser.h"

namespace emptyeisner3rd {
	CombRun::CombRun() = default;

	CombRun::~CombRun() = default;

	void CombRun::parse(const std::string & sInputFile, const std::string & sOutputFile, const std::string & sFeatureFile, const std::string & sECFeatureFile, float lambda) {

		Sentence sentence;
		DependencyTree tree;

		std::cout << "Parsing started" << std::endl;

		auto time_begin = time(NULL);

		std::unique_ptr<CombDepParser> parser(new CombDepParser(sFeatureFile, sECFeatureFile, ParserState::PARSE));
		std::ifstream input(sInputFile);
		std::ofstream output(sOutputFile);
		if (input) {
			while (input >> sentence) {
				if (sentence.size() < MAX_SENTENCE_SIZE) {
					parser->parse(sentence, &tree, lambda);
					output << tree;
					tree.clear();
				}
			}
		}
		input.close();
		output.close();

		auto time_end = time(NULL);

		auto seconds = difftime(time_end, time_begin);

		std::cout << "Parsing has finished successfully. Total time taken is: " << difftime(time_end, time_begin) << "s" << std::endl;
	}
}
