#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include "Server.hpp"
#include "Location.hpp"

class Configuration {
	public :
		int parser(char *configFilePath);
	private :
		bool _extractServerConfigLine(std::string line, Server& current_server);
		bool _extractLocationConfigLine(std::string line, Location& current_location);

		bool hasServerDups();
		std::vector<Server> _Servers;
};