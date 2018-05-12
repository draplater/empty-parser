#ifndef _EMPTY_EISNER_2NDF_COMB_RUN_H
#define _EMPTY_EISNER_2NDF_COMB_RUN_H

#include "common/parser/run_base.h"

namespace emptyeisner2ndf {
	class CombRun {
	public:
		CombRun();
		~CombRun();

		void parse(const std::string & sInputFile, const std::string & sOutputFile, const std::string & sFeatureFile, const std::string & sECFeatureFile, float lambda = -1.0);
	};
}

#endif
