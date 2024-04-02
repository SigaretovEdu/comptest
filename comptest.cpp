#include "comptest.h"

int main(int argc, char* argv[]) {
	Params params = parseArguments(argc, argv);
	params.info();
	checkFiles(params);
	return 0;
}

Params parseArguments(int argc, char* argv[]) {
	Params params;

	for(size_t i = 1; i < argc; ++i) {
		std::string opt(argv[i]);
		std::string last;
		if(argv[i][0] == '-') {
			if(opt == "-q" || opt == "--quite") {
				params.quite = true;
			}
			else if(opt == "-d" || opt == "--disable") {
				params.disableChecking = true;
			}
			else if(opt == "-h" || opt == "--help") {
				usage();
				exit(0);
			}
			else if(opt == "-c" || opt == "--compare") {
				if(i == argc - 1 || (i < argc - 1 && argv[i + 1][0] == '-')) {
					logger[ERROR] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				if(!isCode(argv[i])) {
					logger[ERROR] << "--compare file must be code file\n";
					exit(1);
				}
				params.comparator = argv[i];
			}
			else if(opt == "-g" || opt == "--gen") {
				if(i == argc - 1 || (i < argc - 1 && argv[i + 1][0] == '-')) {
					logger[ERROR] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				if(!isNumber(argv[i])) {
					logger[ERROR] << "Argument for option '" << opt << "' must be integer\n";
					exit(1);
				}
				params.gen = true;
				params.genLimit = std::atoi(argv[i]);
			}
			else if(opt == "-t" || opt == "--test") {
				if(i == argc - 1) {
					logger[ERROR] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				params.exclude = parseTestArg(argv[i], params.tests);
			}
			else {
				logger[ERROR] << "Unknown option '" << opt << "'\n";
				exit(1);
			}
		}
		else {
			if(isTest(opt)) {
				params.testFiles.push_back(opt);
			}
			else if(isCode(opt)) {
				if(params.sourceFile.empty()) {
					params.sourceFile = opt;
				}
				else {
					if(!params.testFiles.empty()) {
						logger[ERROR] << "You specified to much files\n";
						exit(1);
					}
					else {
						params.testFiles.push_back(opt);
					}
				}
			}
		}
	}

	if(params.sourceFile.empty()) {
		logger[ERROR] << "You must specify source file\n";
		exit(1);
	}
	if(params.testFiles.empty()) {
		logger[ERROR] << "You must specify at least one test file\n";
		exit(1);
	}
	if(params.gen && !isCode(params.testFiles[0])) {
		logger[ERROR] << "With --gen option testfile must be code file\n";
		exit(1);
	}
	if(!params.gen) {
		if(isCode(params.testFiles[0])) {
			logger[ERROR] << "Test file cannot be code file\n";
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

bool hasEnding(std::string const& fullString, std::string const& ending) {
	if(fullString.length() >= ending.length()) {
		return (0 ==
				fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool isCode(const std::string s) {
	bool f = false;
	for(auto& it: compileargs) {
		if(hasEnding(s, it.first)) {
			f = true;
			break;
		}
	}
	return f;
}

bool isTest(const std::string& s) {
	return hasEnding(s, ".test");
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
				logger[ERROR] << "Tests ids must be integer\n";
				exit(1);
			}
			tests.push_back(std::atoi(number.c_str()));
			s.erase(0, pos + 1);
			pos = s.find(',');
		}
		if(!isNumber(s)) {
			logger[ERROR] << "Tests ids must be integer\n";
			exit(1);
		}
		tests.push_back(std::atoi(s.c_str()));
	}
	else if(s.find("..") != std::string::npos) {
		size_t pos = s.find(".");
		std::string sub1 = s.substr(0, pos);
		std::string sub2 = s.substr(pos + 2, s.size() - (pos + 2));
		if(!isNumber(sub1) || !isNumber(sub2)) {
			logger[ERROR] << "Tests ids must be integer\n";
			exit(1);
		}
		int num1 = std::atoi(sub1.c_str()), num2 = std::atoi(sub2.c_str());
		for(int i = num1; i <= num2; ++i) {
			tests.push_back(i);
		}
	}
	else {
		if(!isNumber(s)) {
			logger[ERROR] << "Test id must be integer\n";
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
                                                        subject to the same restrictions as sourcefile
)";
	logger[INFO] << usagePrompt;
}

void checkFiles(const Params& params) {
	if(!std::filesystem::exists(params.sourceFile)) {
		logger[ERROR] << params.sourceFile << " doesn't exist\n";
		exit(1);
	}
	for(const auto& s: params.testFiles) {
		if(!std::filesystem::exists(s)) {
			logger[ERROR] << s << " doesn't exist\n";
			exit(1);
		}
	}
	if(!params.comparator.empty() && !std::filesystem::exists(params.comparator)) {
		logger[ERROR] << "Comparator file doesn't exist\n";
		exit(1);
	}
}
