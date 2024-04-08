#pragma once

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <unistd.h>
#include <vector>

#define ll long long
#define ull unsigned long long

struct Params;
class Logger;
enum Loglevel { INFO, ERROR, DEBUG };
class Parser;

class Logger {
  public:
	Logger() {}

	Logger& operator[](Loglevel l) {
		switch(l) {
			case INFO: {
				std::cout << "[INFO ] ";
				break;
			}
			case ERROR: {
				std::cout << "[ERROR] ";
				break;
			}
			case DEBUG: {
				break;
			}
		}
		return *this;
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

	std::string runSource;
	std::string runComparator;
	std::string runTest;

	Params(): exclude(false), quite(false), disableChecking(false), gen(false), genLimit(-1) {}

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
	std::string got;
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
void parseFiles(const Params& params, std::map<int, Test>& tests);
void compile(Params& params);
void prepareFile(const std::string buildname, std::string& filename, std::string& run);
