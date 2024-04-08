#include "comptest.h"

int main(int argc, char* argv[]) {
	Params params = parseArguments(argc, argv);
	params.info();
	checkFiles(params);

	std::map<int, Test> tests;

	parseFiles(params, tests);

	compile(params);

	return 0;
}

Params parseArguments(int argc, char* argv[]) {
	Params params;

	for(int i = 1; i < argc; ++i) {
		std::string opt(argv[i]);
		std::string last;
		if(argv[i][0] == '-') {
			if(opt == "-q" || opt == "--quite") {
				params.quite = true;
			} else if(opt == "-d" || opt == "--disable") {
				params.disableChecking = true;
			} else if(opt == "-h" || opt == "--help") {
				usage();
				exit(0);
			} else if(opt == "-c" || opt == "--compare") {
				if(i == argc - 1 || (i < argc - 1 && argv[i + 1][0] == '-')) {
					logg[ERROR] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				if(!isCode(argv[i])) {
					logg[ERROR] << "--compare file must be code file\n";
					exit(1);
				}
				params.comparator = argv[i];
			} else if(opt == "-g" || opt == "--gen") {
				if(i == argc - 1 || (i < argc - 1 && argv[i + 1][0] == '-')) {
					logg[ERROR] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				if(!isNumber(argv[i])) {
					logg[ERROR] << "Argument for option '" << opt << "' must be integer\n";
					exit(1);
				}
				params.gen = true;
				params.genLimit = std::atoi(argv[i]);
			} else if(opt == "-t" || opt == "--test") {
				if(i == argc - 1) {
					logg[ERROR] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				params.exclude = parseTestArg(argv[i], params.tests);
			} else {
				logg[ERROR] << "Unknown option '" << opt << "'\n";
				exit(1);
			}
		} else {
			if(isTest(opt)) {
				params.testFiles.push_back(opt);
			} else if(isCode(opt)) {
				if(params.sourceFile.empty()) {
					params.sourceFile = opt;
				} else {
					if(!params.testFiles.empty()) {
						logg[ERROR] << "You specified to much files\n";
						exit(1);
					} else {
						params.testFiles.push_back(opt);
					}
				}
			}
		}
	}

	if(params.sourceFile.empty()) {
		logg[ERROR] << "You must specify source file\n";
		exit(1);
	}
	if(params.testFiles.empty()) {
		logg[ERROR] << "You must specify at least one test file\n";
		exit(1);
	}
	if(params.gen && !isCode(params.testFiles[0])) {
		logg[ERROR] << "With --gen option testfile must be code file\n";
		exit(1);
	}
	if(!params.gen) {
		if(isCode(params.testFiles[0])) {
			logg[ERROR] << "Test file cannot be code file\n";
			exit(1);
		}
	}

	return params;
}

bool isNumber(const std::string& s) {
	std::string::const_iterator it = s.begin();
	while(it != s.end() && std::isdigit(*it)) {
		++it;
	}
	return !s.empty() && it == s.end();
}

bool isCode(const std::string s) {
	if(cmds.find(std::filesystem::path(s).extension()) == cmds.end()) {
		return false;
	}
	return true;
}

bool isTest(const std::string& s) {
	return std::filesystem::path(s).extension() == ".test";
}

bool parseTestArg(std::string s, std::vector<int>& tests) {
	bool exclude = false;
	if(s[0] == '-') {
		exclude = true;
		s.erase(0, 1);
	}
	if(s.find(',') != std::string::npos) {
		size_t pos = s.find(',');
		while(pos < s.size()) {
			std::string number = s.substr(0, pos);
			if(!isNumber(number)) {
				logg[ERROR] << "Tests ids must be integer\n";
				exit(1);
			}
			tests.push_back(std::atoi(number.c_str()));
			s.erase(0, pos + 1);
			pos = s.find(',');
		}
		if(!isNumber(s)) {
			logg[ERROR] << "Tests ids must be integer\n";
			exit(1);
		}
		tests.push_back(std::atoi(s.c_str()));
	} else if(s.find("..") != std::string::npos) {
		size_t pos = s.find(".");
		std::string sub1 = s.substr(0, pos);
		std::string sub2 = s.substr(pos + 2, s.size() - (pos + 2));
		if(!isNumber(sub1) || !isNumber(sub2)) {
			logg[ERROR] << "Tests ids must be integer\n";
			exit(1);
		}
		int num1 = std::atoi(sub1.c_str()), num2 = std::atoi(sub2.c_str());
		for(int i = num1; i <= num2; ++i) {
			tests.push_back(i);
		}
	} else {
		if(!isNumber(s)) {
			logg[ERROR] << "Test id must be integer\n";
		}
		int num = std::atoi(s.c_str());
		tests.push_back(num);
	}
	return exclude;
}

void usage() {
	const char* usagePrompt = R"(Usage: comptest [OPTIONS...] sourcefile testsfiles...

    -t, --test testfile:[NUM][NUM,...,NUM][NUM..NUM]    specify tests to use from file, [NUM] will use the only test,
                                                        [NUM,...,NUM] will use tests in specified list, [NUM..NUM]
                                                        will use test in range

    -c, --compare sourcefile                            specify file with another solution, every test will be passed
                                                        to your solution and this file, outputs will be compared

    -q, --quite                                         do not show the outputs of test, only result for every test

    -d, --disable                                       disable comparing output from your solution with
                                                        specified in test file

    -h, --help                                          show this message

    -g, --gen NUM                                       will generate tests using testfile while outputs from source
                                                        file and comparator are equal, with NUM you can specify
                                                        maximum tests to generate, in this case the testfile is
                                                        subject to the same restrictions as sourcefile and you must
                                                        use comparator
)";
	logg[INFO] << usagePrompt;
}

void checkFiles(const Params& params) {
	if(!std::filesystem::exists(params.sourceFile)) {
		logg[ERROR] << params.sourceFile << " doesn't exist\n";
		exit(1);
	}
	for(const auto& s: params.testFiles) {
		if(!std::filesystem::exists(s)) {
			logg[ERROR] << s << " doesn't exist\n";
			exit(1);
		}
	}
	if(!params.comparator.empty() && !std::filesystem::exists(params.comparator)) {
		logg[ERROR] << "Comparator file doesn't exist\n";
		exit(1);
	}
}

void parseFiles(const Params& params, std::map<int, Test>& tests) {
	enum State { WAIT_INPUT, READ_INPUT, READ_OUTPUT };

	State state;
	std::ifstream input;
	std::string line, tmp;
	std::stringstream ss;
	int num;
	for(auto& filepath: params.testFiles) {
		state = WAIT_INPUT;

		input.open(filepath);

		while(std::getline(input, line)) {
			switch(state) {
				case WAIT_INPUT: {
					if(line.size() >= 4 && line.substr(0, 3) == "-- ") {
						ss << line;
						ss >> tmp;
						if(ss >> num) {
							if((params.tests.empty()) ||
							   (!params.exclude &&
								std::find(params.tests.begin(), params.tests.end(), num) !=
									std::end(params.tests)) ||
							   (params.exclude &&
								std::find(params.tests.begin(), params.tests.end(), num) ==
									std::end(params.tests))) {
								tests[num];
								state = READ_INPUT;
							}
						}
						ss.clear();
						tmp.clear();
					}
					break;
				}
				case READ_INPUT: {
					if(line.size() >= 2 && line[0] == '-' && line[1] == '-') {
						tests[num].input = tmp;
						tmp.clear();
						if(line == "--") {
							state = WAIT_INPUT;
						} else {
							state = READ_OUTPUT;
						}
						continue;
					}
					tmp += line + "\n";
					break;
				}
				case READ_OUTPUT: {
					if(line.size() >= 2 && line[0] == '-' && line[1] == '-') {
						tests[num].outputs.push_back(tmp);
						tmp.clear();
						if(line.size() == 2) {
							state = WAIT_INPUT;
						}
						continue;
					}
					tmp += line + "\n";
					break;
				}
			}
		}

		input.close();
	}
}

void compile(Params& params) {
	using namespace std::filesystem;

	std::string buildname = path(params.sourceFile).stem();
	if(!exists(buildname) && create_directory(buildname) != true) {
		logg[ERROR] << "Failed to create " << buildname << " directory\n";
		exit(1);
	}

	prepareFile(buildname, params.sourceFile, params.runSource);
	if(!params.comparator.empty()) {
		prepareFile(buildname, params.comparator, params.runComparator);
	}
	if(params.gen) {
		prepareFile(buildname, params.testFiles[0], params.runTest);
	}

	params.info();
}

void prepareFile(const std::string buildname, std::string& filename, std::string& run) {
	using namespace std::filesystem;
	std::string extension = path(filename).extension();
	if(!cmds[extension].first.empty()) {
		std::string name = buildname + "/" + std::string(path(filename).stem());
		std::system((cmds[extension].first + " " + name + " " + filename).c_str());
		run = cmds[extension].second;
		if(run.empty()) {
			run += name;
		} else {
			run += " " + name;
		}
	} else {
		run = cmds[extension].second;
		if(run.empty()) {
			run += filename;
		} else {
			run += " " + filename;
		}
	}
}

// void runTests(Test& test, std::string executable, ) {
// 	int fdToChild[2];
// 	int fdToParent[2];
//
// 	pipe(fdToChild);
// 	pipe(fdToParent);
//
// 	switch(fork()) {
// 		case -1: {
// 			logg[ERROR] << "Failed to fork\n";
// 			exit(1);
// 			break;
// 		}
// 		case 0: {
// 			break;
// 		}
// 		default: {
// 			break;
// 		}
// 	}
// }
