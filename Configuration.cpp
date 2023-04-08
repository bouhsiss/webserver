#include "Configuration.hpp"

void trimSpaces(std::string &line) {
	line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
	line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
}

void Configuration::parser(char *configFilePath) {
	std::string conf(configFilePath);
	std::ifstream file(conf);
	std::string line;

	bool inServer = false;
	bool inLocation = false;

	Server currentServer;
	Location currentLocation;

	if(!file.is_open()) {
		std::cout << "couldn't open file" << std::endl;
		return;
	}
	while(std::getline(file, line)) {
		//strip comments if found
		int pos = line.find("#");
		if(pos != std::string::npos)
			line = line.substr(0, pos);

		trimSpaces(line);

		if(line.empty())
			continue ;
		int openedBraces = std::count(line.begin(), line.end(), '{');
		int closedBraces = std::count(line.begin(), line.end(), '}');
		int braceCount = openedBraces - closedBraces;

		if(line.find("server") != std::string::npos) {
			inServer = true;
			currentServer = Server();
		}
		else if(line.find("location") != std::string::npos) {
			inLocation = true;
			currentLocation = Location();
			int pathStart = line.find_first_of("/");
			int pathEnd = line.find_last_of("{");
			currentLocation.path = line.substr(pathStart, pathEnd - pathStart);
		}
		else if(inLocation && braceCount == 0) {
			inLocation = false;
			currentServer._Locations.push_back(currentLocation);
		}
		else if(inServer && braceCount == 0) {
			inServer = false;
			_Servers.push_back(currentServer);
		}
		else if(inServer) {
			//parse server lines in current_server
		}
		else if(inLocation) {
			//parse Location lines in current_server
		}
	}
}