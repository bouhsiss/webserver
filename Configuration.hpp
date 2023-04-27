#pragma once
#include "Http.hpp"
#include "Server.hpp"

class Configuration {
	public :
		std::vector<Server> parse(std::string configFilePath);

	private :
		bool _extractServerConfigLine(std::string line, Server& current_server);
		bool _extractLocationConfigLine(std::string line, Location& current_location);

		bool serverHasDups();
};

