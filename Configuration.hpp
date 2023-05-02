#pragma once
#include "Http.hpp"
#include "Server.hpp"

class Configuration {
	public :
		std::vector<Server> parse(std::string configFilePath);

	private :
		bool _extractServerConfigLine(std::string line, Server& current_server);
		bool _extractLocationConfigLine(std::string line, Location& current_location);
		void areServersDuplicated();
};

// add a function to check for duplicates when host and ports are similar
// add to my parsing a check for validity of values 
// check for validity of config file
// assing default values if listen and port are missing and allowed methods
// check if there's a duplication of directives or empty directives
// loop over servers and for each server's host:port creat a socket and store
