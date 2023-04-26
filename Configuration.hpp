#pragma once
#include "Http.hpp"
#include "Server.hpp"

class Configuration {
	public :
		int parse(std::string configFilePath, std::vector<Server> &servers);

	private :
		bool _extractServerConfigLine(std::string line, Server& current_server);
		bool _extractLocationConfigLine(std::string line, Location& current_location);

		bool serverHasDups();
};

