#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<set>
#include<fstream>
#include<map>


#define DEFAULT_CONFIG_FILE "default.conf"
namespace Http {
	void tokenize(std::string const &str, const char* delim, std::vector<std::string>& out);
	void trimSpaces(std::string &line);
}