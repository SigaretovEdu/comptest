#include "comptest.h"

int main(int argc, char* argv[]) {
	Params params = parseArguments(argc, argv);

	// params.info();

	checkFiles(params);

	std::map<int, Test> tests;

	parseFiles(params, tests);

	{
		bool f = false;
		for(int& num: params.tests) {
			if(!params.exclude && tests.find(num) == tests.end()) {
				logg[ER] << "There is no test with id " << num << "\n";
				f = true;
			}
		}
		if(f) {
			exit(1);
		}
	}

	compile(params);

	Result result;

	if(params.comparator.empty()) {
		result = runTests(params, tests);
	}

	printSummary(result);

	clean(params);

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
					logg[ER] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				if(!isCode(argv[i])) {
					logg[ER] << "--compare file must be code file\n";
					exit(1);
				}
				params.comparator = argv[i];
			} else if(opt == "-g" || opt == "--gen") {
				if(i == argc - 1 || (i < argc - 1 && argv[i + 1][0] == '-')) {
					logg[ER] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}

				++i;
				if(!isNumber(argv[i])) {
					logg[ER] << "Argument for option '" << opt << "' must be integer\n";
					exit(1);
				}
				params.gen = true;
				params.genLimit = std::atoi(argv[i]);
			} else if(opt == "-t" || opt == "--test") {
				if(i == argc - 1) {
					logg[ER] << "Option '" << opt << "' requires argument\n";
					exit(1);
				}
				++i;
				params.exclude = parseTestArg(argv[i], params.tests);
			} else {
				logg[ER] << "Unknown option '" << opt << "'\n";
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
						logg[ER] << "You specified to much files\n";
						exit(1);
					} else {
						params.testFiles.push_back(opt);
					}
				}
			}
		}
	}

	if(params.sourceFile.empty()) {
		logg[ER] << "You must specify source file\n";
		exit(1);
	}
	if(params.testFiles.empty()) {
		logg[ER] << "You must specify at least one test file\n";
		exit(1);
	}
	if(params.gen && !isCode(params.testFiles[0])) {
		logg[ER] << "With --gen option testfile must be code file\n";
		exit(1);
	}
	if(!params.gen) {
		if(isCode(params.testFiles[0])) {
			logg[ER] << "Test file cannot be code file\n";
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
				logg[ER] << "Tests ids must be integer\n";
				exit(1);
			}
			tests.push_back(std::atoi(number.c_str()));
			s.erase(0, pos + 1);
			pos = s.find(',');
		}
		if(!isNumber(s)) {
			logg[ER] << "Tests ids must be integer\n";
			exit(1);
		}
		tests.push_back(std::atoi(s.c_str()));
	} else if(s.find("..") != std::string::npos) {
		size_t pos = s.find(".");
		std::string sub1 = s.substr(0, pos);
		std::string sub2 = s.substr(pos + 2, s.size() - (pos + 2));
		if(!isNumber(sub1) || !isNumber(sub2)) {
			logg[ER] << "Tests ids must be integer\n";
			exit(1);
		}
		int num1 = std::atoi(sub1.c_str()), num2 = std::atoi(sub2.c_str());
		for(int i = num1; i <= num2; ++i) {
			tests.push_back(i);
		}
	} else {
		if(!isNumber(s)) {
			logg[ER] << "Test id must be integer\n";
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
		logg[ER] << params.sourceFile << " doesn't exist\n";
		exit(1);
	}
	for(const auto& s: params.testFiles) {
		if(!std::filesystem::exists(s)) {
			logg[ER] << s << " doesn't exist\n";
			exit(1);
		}
	}
	if(!params.comparator.empty() && !std::filesystem::exists(params.comparator)) {
		logg[ER] << "Comparator file doesn't exist\n";
		exit(1);
	}
}

void parseFiles(Params& params, std::map<int, Test>& tests) {
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
					if(line.size() >= 2 && line.substr(0, 2) == "TL") {
						ss << line;
						int tl;
						ss >> tmp >> tmp >> tl;
						ss.clear();
						params.tl = std::max<int>(1, tl);
					}
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
	params.buildname = buildname;
	if(!exists(buildname) && create_directory(buildname) != true) {
		logg[ER] << "Failed to create " << buildname << " directory\n";
		exit(1);
	}

	prepareFile(buildname, params.sourceFile, params.runSource);
	if(!params.comparator.empty()) {
		prepareFile(buildname, params.comparator, params.runComparator);
	}
	if(params.gen) {
		prepareFile(buildname, params.testFiles[0], params.runTest);
	}

	// params.info();
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

void startTest(int tl, const Test& test, const std::string& executable, RunInfo& info) {
	int toChild[2];
	int toParent[2];
	if(pipe(toChild) == -1) {
		logg[ER] << "Failed to create pipe\n";
		exit(1);
	}
	if(pipe(toParent) == -1) {
		logg[ER] << "Failed to create pipe\n";
		exit(1);
	}

	int id = fork();
	switch(id) {
		case -1: {
			logg[ER] << "Failed to fork\n";
			exit(1);
			break;
		}
		case 0: {
			// logg[INFO] << "started " << getpid() << "\n";
			dup2(toChild[0], STDIN_FILENO);
			close(toChild[0]);
			close(toChild[1]);
			dup2(toParent[1], STDOUT_FILENO);
			close(toParent[0]);
			close(toParent[1]);

			alarm(tl);

			if(execve(executable.c_str(), nullptr, nullptr) == -1) {
				logg[ER] << "Failed to execute " << executable << "\n";
				exit(1);
			}

			break;
		}
		default: {
			info.id = id;
			close(toChild[0]);
			close(toParent[1]);
			break;
		}
	}

	write(toChild[1], &test.input.front(), test.input.size());

	close(toChild[1]);

	info.fd = toParent[0];
}

void readPipe(int fd, std::string& text) {
	text.clear();
	char c;
	while(read(fd, &c, 1) > 0) {
		if(c == '\n') {
			deleteTrailing(text);
		}
		text += c;
	}
}

Result runTests(const Params& params, const std::map<int, Test>& tests) {
	std::map<int, RunInfo> fds;
	for(const auto& [num, test]: tests) {
		startTest(params.tl, test, params.runSource, fds[num]);
	}

	std::string output;
	Result result;

	Mode res;
	for(const auto [num, info]: fds) {
		int status;
		res = OK;

		waitpid(info.id, &status, 0);

		if(status != 0) {
			res = ER;
			if(status == 14) {
				res = TL;
			}
		}

		readPipe(info.fd, output);
		close(info.fd);

		if(res == OK) {
			if(!tests.at(num).outputs.empty() && !params.disableChecking) {
				if(!checkOutput(output, tests.at(num).outputs)) {
					res = WA;
				}
			} else {
				res = UK;
			}
		}

		if(!params.quite && res != OK) {
			logg[res] << num << "\n";
			logg[DEBUG] << "-- input\n";
			logg.clear();
			logg[DEBUG] << tests.at(num).input;
			logg.set(res);
			logg[DEBUG] << "-- output\n";
			if(!params.disableChecking) {
				printDiff(params, output, tests.at(num).outputs[0]);
			} else {
				logg.clear();
				logg[DEBUG] << output;
			}
			logg.set(res);
			logg[DEBUG] << "--\n\n";
		}
		result(res, num);
	}

	return result;
}

bool checkOutput(const std::string& got, const std::vector<std::string>& outputs) {
	for(const auto& s: outputs) {
		if(got == s) {
			return true;
		}
	}
	return false;
}

void printDiff(const Params& params, const std::string& left, const std::string& right) {
	std::ofstream leftfile(params.buildname + "/left.txt",
						   std::ios_base::out | std::ios_base::trunc);
	std::ofstream rightfile(params.buildname + "/right.txt",
							std::ios_base::out | std::ios_base::trunc);

	leftfile << left;
	rightfile << right;

	leftfile.close();
	rightfile.close();

	logg.clear();

	std::system(("icdiff --no-headers --whole-file --color-map=change:red_bold " +
				 params.buildname + "/left.txt " + params.buildname + "/right.txt")
					.c_str());
}

void clean(const Params& params) {
	std::filesystem::remove_all(params.buildname);
}

void printSummary(const Result& result) {
	// result.info();
	logg[DEBUG] << "\n";

	printCategory(OK, result.ok);
	if(!result.er.empty()) {
		printCategory(ER, result.er);
	}
	if(!result.tl.empty()) {
		printCategory(TL, result.tl);
	}
	if(!result.wa.empty()) {
		printCategory(WA, result.wa);
	}
	if(!result.uk.empty()) {
		printCategory(UK, result.uk);
	}
}

void printCategory(Mode mode, const std::vector<int>& nums) {
	logg[mode] << "total(" << nums.size() << ") : ";
	for(size_t i = 0; i < nums.size(); ++i) {
		if(i != 0) {
			logg[DEBUG] << ", ";
		}
		logg[DEBUG] << nums[i];
	}
	logg[DEBUG] << "\n";
}

void deleteTrailing(std::string& s) {
	while(!s.empty() && s.back() == ' ') {
		s.pop_back();
	}
}
