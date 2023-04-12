#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include "Server.hpp"
#include "Location.hpp"
#include "Http.hpp"


class Configuration {
	public :
		int parser(char *configFilePath);

		const std::vector<Server>& getServers();
	private :
		bool _extractServerConfigLine(std::string line, Server& current_server);
		bool _extractLocationConfigLine(std::string line, Location& current_location);

		bool serverHasDups();
		std::vector<Server> _Servers;
};

std::ostream& operator<<(std::ostream &out, Configuration& c);