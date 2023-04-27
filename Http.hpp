#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<set>
#include<fstream>
#include<map>
#include <exception>

#define DEFAULT_CONFIG_FILE "default.conf"


#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"
#define YELLOW "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

namespace Http {
	// custom exception for config file parsing errors. 
	class ConfigFileErrorException : public std::runtime_error {
	public :
		ConfigFileErrorException(std::string msg) : std::runtime_error(msg) {}
	};

	void tokenize(std::string const &str, const char* delim, std::vector<std::string>& out);
	void trimSpaces(std::string &line);
}