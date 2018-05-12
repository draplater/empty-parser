#include <memory>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "common/parser/macros_base.h"

#include "common/parser/graph_dp/eisner2nd/eisner2nd_run.h"
#include "common/parser/graph_dp/eisner3rd/eisner3rd_run.h"
#include "common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_run.h"
#include "common/parser/graph_dp/emptyeisner2nd/emptyeisner2nd_combrun.h"
#include "common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_run.h"
#include "common/parser/graph_dp/emptyeisner2ndf/emptyeisner2ndf_combrun.h"
#include "common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_run.h"
#include "common/parser/graph_dp/emptyeisner3rd/emptyeisner3rd_combrun.h"

#include "common/parser/graph_dd/dual_decomposition2nd.h"
#include "common/parser/graph_dd/dual_decomposition2ndf.h"
#include "common/parser/graph_dd/dual_decomposition3rd.h"
#include "common/parser/graph_dd/dual_decomposition2ec3rd.h"
#include "common/parser/graph_dd/dual_decomposition3ec2nd.h"

#define	SLASH	"/"

int main(int argc, char * argv[]) {

	std::ios_base::sync_with_stdio(false);
	std::cin.tie(NULL);
	std::cout << std::fixed << std::setprecision(4);

	std::unique_ptr<RunBase> run(nullptr);

	if (strcmp(argv[2], "eisner2nd") == 0) {
		run.reset(new eisner2nd::Run());
	}
	else if (strcmp(argv[2], "eisner3rd") == 0) {
		run.reset(new eisner3rd::Run());
	}
	else if (strcmp(argv[2], "emptyeisner2nd") == 0) {
		run.reset(new emptyeisner2nd::Run());
	}
	else if (strcmp(argv[2], "emptyeisner2ndf") == 0) {
		run.reset(new emptyeisner2ndf::Run());
	}
	else if (strcmp(argv[2], "emptyeisner3rd") == 0) {
		run.reset(new emptyeisner3rd::Run());
	}

	if (strcmp(argv[1], "goldtest") == 0) {
		run->goldtest(argv[3], argv[4]);
	}
	else if (strcmp(argv[1], "train") == 0) {

		int iteration = std::atoi(argv[5]);

		std::string current_feature;
		std::string next_feature;

		current_feature = next_feature = argv[4];
		next_feature = next_feature.substr(0, next_feature.rfind(SLASH) + strlen(SLASH)) + argv[2] + "1.feat";

		for (int i = 0; i < iteration; ++i) {
			run->train(argv[3], current_feature, next_feature);
			current_feature = next_feature;
			next_feature = next_feature.substr(0, next_feature.rfind(argv[2]) + strlen(argv[2])) + std::to_string(i + 2) + ".feat";
		}
	}
	else if (strcmp(argv[1], "parse") == 0) {
		run->parse(argv[3], argv[5], argv[4]);
	}
	else if (strcmp(argv[1], "dd2nd") == 0) {
		DualDecomposition2nd dd;
		std::string name1 = argv[4];
		std::string name2 = argv[5];
		std::string name3 = argv[6];
		std::ifstream ifs1(name1);
		std::ifstream ifs2(name2);
		DependencyTree ret_tree;
		DependencyTree tree1, tree2;
		if (argc == 7) {
			std::ofstream ofs(name3);
			dd.init(argv[2], argv[3], .5e6, 200);
			while (ifs1 >> tree1) {
				ifs2 >> tree2;
				dd.decode(tree1, tree2, ret_tree);
				ofs << ret_tree;
			}
			dd.stat();
		}
		else {
			float lambda = atof(argv[7]);
			name3 += argv[7];
			std::ofstream ofs(name3);
			dd.init(argv[2], argv[3], .5e6 * std::fmin(lambda, 1.0 - lambda), 200);
			while (ifs1 >> tree1) {
				ifs2 >> tree2;
				dd.decode(tree1, tree2, ret_tree, lambda);
				ofs << ret_tree;
			}
			dd.stat();
		}
	}
	else if (strcmp(argv[1], "dd2ndf") == 0) {
		DualDecomposition2ndf dd;
		std::string name1 = argv[4];
		std::string name2 = argv[5];
		std::string name3 = argv[6];
		std::ifstream ifs1(name1);
		std::ifstream ifs2(name2);
		DependencyTree ret_tree;
		DependencyTree tree1, tree2;
		if (argc == 7) {
			std::ofstream ofs(name3);
			dd.init(argv[2], argv[3], .5e6, 200);
			while (ifs1 >> tree1) {
				ifs2 >> tree2;
				dd.decode(tree1, tree2, ret_tree);
				ofs << ret_tree;
			}
		}
		else {
			float lambda = atof(argv[7]);
			name3 += argv[7];
			std::ofstream ofs(name3);
			dd.init(argv[2], argv[3], .5e6 * std::fmin(lambda, 1.0 - lambda), 200);
			while (ifs1 >> tree1) {
				ifs2 >> tree2;
				dd.decode(tree1, tree2, ret_tree, lambda);
				ofs << ret_tree;
			}
		}
	}
	else if (strcmp(argv[1], "dd3rd") == 0) {
		DualDecomposition3rd dd;
		std::string name1 = argv[4];
		std::string name2 = argv[5];
		std::string name3 = argv[6];
		std::ifstream ifs1(name1);
		std::ifstream ifs2(name2);
		DependencyTree ret_tree;
		DependencyTree tree1, tree2;
		if (argc == 7) {
			std::ofstream ofs(name3);
			dd.init(argv[2], argv[3], .5e6, 200);
			while (ifs1 >> tree1) {
				ifs2 >> tree2;
				dd.decode(tree1, tree2, ret_tree);
				ofs << ret_tree;
			}
		}
		else {
			float lambda = atof(argv[7]);
			name3 += argv[7];
			std::ofstream ofs(name3);
			dd.init(argv[2], argv[3], .5e6 * std::fmin(lambda, 1.0 - lambda), 200);
			while (ifs1 >> tree1) {
				ifs2 >> tree2;
				dd.decode(tree1, tree2, ret_tree, lambda);
				ofs << ret_tree;
			}
		}
	}
	else if (strcmp(argv[1], "dd2ec3rd") == 0) {
		DualDecomposition2ec3rd dd;
		dd.init(argv[2], argv[3], .5e6, 200);
		std::string name1 = argv[4];
		std::string name2 = argv[5];
		std::string name3 = argv[6];
		std::ifstream ifs1(name1);
		std::ifstream ifs2(name2);
		std::ofstream ofs(name3);
		DependencyTree ret_tree;
		DependencyTree tree1, tree2;
		while (ifs1 >> tree1) {
			ifs2 >> tree2;
			dd.decode(tree1, tree2, ret_tree);
			ofs << ret_tree;
		}
	}
	else if (strcmp(argv[1], "dd3ec2nd") == 0) {
		DualDecomposition3ec2nd dd;
		dd.init(argv[2], argv[3], .5e6, 200);
		std::string name1 = argv[4];
		std::string name2 = argv[5];
		std::string name3 = argv[6];
		std::ifstream ifs1(name1);
		std::ifstream ifs2(name2);
		std::ofstream ofs(name3);
		DependencyTree ret_tree;
		DependencyTree tree1, tree2;
		while (ifs1 >> tree1) {
			ifs2 >> tree2;
			dd.decode(tree1, tree2, ret_tree);
			ofs << ret_tree;
		}
	}
	else if (strcmp(argv[1], "comb2nd") == 0) {
		std::unique_ptr<emptyeisner2nd::CombRun> run(nullptr);
		run.reset(new emptyeisner2nd::CombRun());
		if (argc == 7) {
			std::string output = argv[5];
			output += argv[6];
			float lambda = atof(argv[7]);
			run->parse(argv[4], output, argv[2], argv[3], lambda);
		}
		else {
			run->parse(argv[4], argv[5], argv[2], argv[3]);
		}
	}
	else if (strcmp(argv[1], "comb2ndf") == 0) {
		std::unique_ptr<emptyeisner2ndf::CombRun> run(nullptr);
		run.reset(new emptyeisner2ndf::CombRun());
		if (argc == 7) {
			std::string output = argv[5];
			output += argv[6];
			float lambda = atof(argv[7]);
			run->parse(argv[4], output, argv[2], argv[3], lambda);
		}
		else {
			run->parse(argv[4], argv[5], argv[2], argv[3]);
		}
	}
	else if (strcmp(argv[1], "comb3rd") == 0) {
		std::unique_ptr<emptyeisner3rd::CombRun> run(nullptr);
		run.reset(new emptyeisner3rd::CombRun());
		if (argc == 7) {
			std::string output = argv[5];
			output += argv[6];
			float lambda = atof(argv[7]);
			run->parse(argv[4], output, argv[2], argv[3], lambda);
		}
		else {
			run->parse(argv[4], argv[5], argv[2], argv[3]);
		}
	}
	else if (strcmp(argv[1], "test") == 0) {
		std::string name1 = argv[2];
		std::string name2 = argv[3];
		std::ifstream ifs(name1);
		std::ofstream ofs(name2);
		DependencyTree tree, ret_tree;
		while (ifs >> tree) {
			ret_tree = emptyToNonEmpty(tree);
			ofs << ret_tree;
		}
	}
}
