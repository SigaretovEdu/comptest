#pragma once

#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#define ll long long
#define ull unsigned long long

struct Params;
class Logger;
enum Mode { INFO = 0, DEBUG = 1, OK = 2, ER = 3, TL = 4, WA = 5, UK = 6 };
class Parser;

const std::map<std::string, std::string> colors = {
	{"clear", "\033[0;0;0m"}, {"er", "\033[31m"}, {"ok", "\033[32m"},
	{"uk", "\033[33m"},		  {"tl", "\033[31m"},	 {"wa", "\033[31m"},
};
class Logger {
  private:

      Mode last;

  public:
	Logger() {}

	Logger& operator[](Mode l) {
        this->set(l);
		switch(l) {
			case INFO: {
				std::cout << "[INFO] ";
				break;
			}
			case ER: {
				std::cout << "[ER] ";
				break;
			}
			case OK: {
				std::cout << "[OK] ";
				break;
			}
			case UK: {
				std::cout << "[UK] ";
				break;
			}
			case TL: {
				std::cout << "[TL] ";
				break;
			}
			case WA: {
				std::cout << "[WA] ";
				break;
			}
			case DEBUG: {
				break;
			}
		}
		return *this;
	}

	void clear() {
		std::cout << colors.at("clear");
		std::cout.flush();
	}

    void set(Mode mode) {
		switch(mode) {
			case INFO: {
				std::cout << colors.at("clear");
				break;
			}
			case ER: {
				std::cout << colors.at("er");
				break;
			}
			case OK: {
				std::cout << colors.at("ok");
				break;
			}
			case UK: {
				std::cout << colors.at("uk");
				break;
			}
			case TL: {
				std::cout << colors.at("tl");
				break;
			}
			case WA: {
				std::cout << colors.at("wa");
				break;
			}
			case DEBUG: {
				break;
			}
		}
        std::cout.flush();
    }

	template<typename T>
	Logger& operator<<(const T& s) {
		std::cout << s;
		return *this;
	}
};
static Logger logg;

struct Params {
	std::string sourceFile;
	std::vector<std::string> testFiles;
	std::string comparator;
	std::vector<int> tests;
	bool exclude;
	bool quite;
	bool disableChecking;
	bool gen;
	int genLimit;

	int tl;

	std::string buildname;
	std::string runSource;
	std::string runComparator;
	std::string runTest;

	Params():
		exclude(false), quite(false), disableChecking(false), gen(false), genLimit(-1), tl(1) {}

	void info() const {
		logg[INFO] << "sourceFile: " << sourceFile << "\n";
		logg[INFO] << "testFiles: " << testFiles.size() << "\n";
		for(size_t i = 0; i < testFiles.size(); ++i) {
			logg[INFO] << "\t" << testFiles[i] << "\n";
		}
		logg[INFO] << "comparator: " << comparator << "\n";
		logg[INFO] << "tests: ";
		for(size_t i = 0; i < tests.size(); ++i) {
			logg[DEBUG] << tests[i] << " ";
		}
		logg[DEBUG] << "\n";
		logg[INFO] << "exclude: " << ((exclude) ? "true" : "false") << "\n";
		logg[INFO] << "quite: " << ((quite) ? "true" : "false") << "\n";
		logg[INFO] << "disableChecking: " << ((disableChecking) ? "true" : "false") << "\n";
		logg[INFO] << "gen: " << ((gen) ? "true" : "false") << "\n";
		logg[INFO] << "genLimit: " << genLimit << "\n";
		logg[INFO] << "tl: " << tl << "\n";
		logg[INFO] << "buildname: " << buildname << "\n";
		logg[INFO] << "runSource: " << runSource << "\n";
		logg[INFO] << "runTest: " << runTest << "\n";
		logg[INFO] << "runComparator: " << runComparator << "\n";
		logg[DEBUG] << "\n";
	}
};

static std::map<std::string, std::pair<std::string, std::string>> cmds = {
	{".cpp", {"g++ -Wall -Wextra -Wpedantic -g -O2 -o", ""}},
	{".c", {"gcc -Wall -Wextra -Wpedantic -g -O2 -o", ""}},
	{".py", {"", "python3"}},
};

struct Test {
	size_t num;
	std::string input;
	std::vector<std::string> outputs;
};

struct RunInfo {
	pid_t id;
	int fd;
};

struct Result {
	std::vector<int> ok;
	std::vector<int> er;
	std::vector<int> tl;
	std::vector<int> wa;
	std::vector<int> uk;

	void operator()(Mode res, int num) {
		switch(res) {
			case OK: {
				ok.push_back(num);
				break;
			}
			case ER: {
				er.push_back(num);
				break;
			}
			case TL: {
				tl.push_back(num);
				break;
			}
			case WA: {
				wa.push_back(num);
				break;
			}
			case UK: {
				uk.push_back(num);
				break;
			}
			default: {
				logg[ER] << "Unknown mode\n";
				exit(1);
			}
		}
	}

	void info() const {
		logg[INFO] << "ok: " << ok.size() << "\n";
		logg[INFO] << "er: " << er.size() << "\n";
		logg[INFO] << "tl: " << tl.size() << "\n";
		logg[INFO] << "wa: " << wa.size() << "\n";
		logg[INFO] << "uk: " << uk.size() << "\n";
	}
};

Params parseArguments(int argc, char* argv[]);
bool isNumber(const std::string& s);
bool hasEnding(std::string const& fullString, std::string const& ending);
bool isCode(const std::string s);
bool isTest(const std::string& s);
bool parseTestArg(std::string s, std::vector<int>& tests);
void usage();
bool fileExist(const std::string& path);
void checkFiles(const Params& params);
void parseFiles(Params& params, std::map<int, Test>& tests);
void compile(Params& params);
void prepareFile(const std::string buildname, std::string& filename, std::string& run);
void startTest(int tl, const Test& test, const std::string& executable, RunInfo& info);
void readPipe(int fd, std::string& text);
Result runTests(const Params& params, const std::map<int, Test>& tests);
bool checkOutput(const std::string& got, const std::vector<std::string>& outputs);
void printDiff(const Params& params, const std::string& left, const std::string& right);
void clean(const Params& params);
void printSummary(const Result& result);
void printCategory(Mode mode, const std::vector<int>& nums);
void deleteTrailing(std::string& s);
