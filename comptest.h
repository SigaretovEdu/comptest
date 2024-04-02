#pragma once

#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

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
static Logger logger;

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

	Params(): exclude(false), quite(false), disableChecking(false), gen(false), genLimit(-1) {}

	void info() const {
		logger[INFO] << "sourceFile: " << sourceFile << "\n";
		logger[INFO] << "testFiles: " << testFiles.size() << "\n";
		for(size_t i = 0; i < testFiles.size(); ++i) {
			logger[INFO] << "\t" << testFiles[i] << "\n";
		}
		logger[INFO] << "comparator: " << comparator << "\n";
		logger[INFO] << "tests: ";
		for(size_t i = 0; i < tests.size(); ++i) {
			logger[DEBUG] << tests[i] << " ";
		}
		logger[DEBUG] << "\n";
		logger[INFO] << "exclude: " << ((exclude) ? "true" : "false") << "\n";
		logger[INFO] << "quite: " << ((quite) ? "true" : "false") << "\n";
		logger[INFO] << "disableChecking: " << ((disableChecking) ? "true" : "false") << "\n";
		logger[INFO] << "gen: " << ((gen) ? "true" : "false") << "\n";
		logger[INFO] << "genLimit: " << genLimit << "\n";
	}
};

std::map<std::string, std::vector<std::string>> compileargs = {
	{"cpp", {"g++", "-Wall", "-Wextra", "-Wpedantic", "-g"}},
	{"c", {"gcc", "-Wall", "-Wextra", "-Wpedantic", "-g"}},
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
