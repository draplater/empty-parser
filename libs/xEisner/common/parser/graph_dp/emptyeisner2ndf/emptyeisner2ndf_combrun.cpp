#include <ctime>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>

#include "emptyeisner2ndf_combrun.h"
#include "emptyeisner2ndf_combdepparser.h"

namespace emptyeisner2ndf {
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
				tree.clear();
				parser->parse(sentence, &tree, lambda);
				if (!tree.empty()) {
					output << tree;
				}
			}
		}
		input.close();
		output.close();

		std::cout << "Parsing has finished successfully. Total time taken is: " << difftime(time(NULL), time_begin) << "s" << std::endl;

	}
}
