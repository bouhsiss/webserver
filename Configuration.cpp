#include "Configuration.hpp"

bool Configuration::_extractServerConfigLine(std::string line, Server& current_server) {
	int EqualSignPos = line.find('=');
	if(EqualSignPos == std::string::npos)
		return(false);
	std::string key = line.substr(0, EqualSignPos);
	std::string value = line.substr(EqualSignPos + 1);
	Http::trimSpaces(key);
	Http::trimSpaces(value);
	if(key == "port")
		current_server._port = value;
	else if(key == "server_name")
		Http::tokenize(value, " ", current_server._server_name);
	else if(key == "host")
		current_server._host = value;
	else if(key == "error_page")
		Http::tokenize(value, " ", current_server._error_page);
	else if(key == "client_body_size_limit")
		current_server._client_body_size_limit = std::stoi(value);
	else
		return(false);
	return(true);
}

bool Configuration::_extractLocationConfigLine(std::string line, Location& current_location) {
	int EqualSignPos = line.find('=');
	if(EqualSignPos == std::string::npos)
		return(false);
	std::string key = line.substr(0, EqualSignPos);
	std::string value = line.substr(EqualSignPos + 1);
	Http::trimSpaces(key);
	Http::trimSpaces(value);
	if(key == "path")
		current_location._path = value;
	else if (key == "allowed_methods")
		Http::tokenize(value, " ", current_location._allowed_methods);
	else if(key == "root")
		current_location._root = value;
	else if(key == "index")
		Http::tokenize(value, " ", current_location._index);
	else if(key == "redirect")
		current_location._redirect = value;
	else if(key == "autoindex")
		current_location._autoindex = value;
	else if(key == "upload_path")
		current_location._upload_path = value;
	else if(key == "cgi_extension")
		current_location._cgi_extension = value;
	else if(key == "cgi_path")
		current_location._cgi_path = value;
	else
		return(false);
	return(true);
}

int Configuration::parser(char *configFilePath) {
	std::string conf(configFilePath);
	std::ifstream file(conf);
	std::string line;

	bool inServer = false;
	bool inLocation = false;

	Server currentServer;
	Location currentLocation;

	int openedBraces = 0;
	int closedBraces = 0;
	int braceCount = 0;

	if(!file.is_open()) {
		std::cout << "couldn't open file" << std::endl;
		return(EXIT_FAILURE);
	}
	while(std::getline(file, line)) {
		//strip comments if found
		int pos = line.find("#");
		if(pos != std::string::npos)
			line = line.substr(0, pos);

		Http::trimSpaces(line);

		openedBraces = std::count(line.begin(), line.end(), '{');
		closedBraces = std::count(line.begin(), line.end(), '}');
		braceCount +=  (openedBraces - closedBraces);

		if(line.empty())
			continue ;
		
		if(line.find("server {") != std::string::npos) {
			/*---------- check if the server block is valid*/
			if(braceCount != 1) {
				std::cout << "missing curly brace in server block" << std::endl;
				return(EXIT_FAILURE);
			}
			if(inServer || inLocation) {
				std::cout << "server block Invalid inheritance" << std::endl;
				return(EXIT_FAILURE);
			}
			inServer = true;
			currentServer = Server();
		}
		else if(line.find("location /") != std::string::npos) {
			/*---------- check if the location block is valid*/
			if(braceCount != 2) {
				std::cout << "missing curly brace in location block" << std::endl;
				return(EXIT_FAILURE);
			}
			if(inLocation == true || inServer == false) {
				std::cout << "location block Invalid inheritance " << std::endl;
				std::cout << line << std::endl;
				std::cout << currentServer << std::endl;
				return(EXIT_FAILURE);
			}
			inLocation = true;
			currentLocation = Location();
			int pathStart = line.find_first_of("/");
			int pathEnd = line.find_last_of("{");
			currentLocation._path = line.substr(pathStart, pathEnd - pathStart);
		}
		else if(inLocation && braceCount == 1) {
			inLocation = false;
			currentServer._Locations.push_back(currentLocation);
		}
		else if(inServer && braceCount == 0) {
			inServer = false;
			this->_Servers.push_back(currentServer);
		}
		else if(inServer && !inLocation) {
			if(!_extractServerConfigLine(line, currentServer)) {
				std::cout << "invalid server block config line" << std::endl;
				std::cout << line << std::endl;
				return(EXIT_FAILURE);
			}
		}
		else if(inLocation) {
			if(!_extractLocationConfigLine(line, currentLocation)) {
				std::cout << "invalid location block config line" << std::endl;
				return(EXIT_FAILURE);
			}
		}
		else {
			std::cout << "invalid syntax" << std::endl;
			return(EXIT_FAILURE);
		}
	}
	return(EXIT_SUCCESS);
}

bool Configuration::serverHasDups() {
	std::set<std::pair<std::string, std::string> > noDups;
	std::vector<Server>::const_iterator It;
	for(It = _Servers.begin(); It != _Servers.end(); It++) {
		std::pair<std::set<std::pair<std::string, std::string> >::iterator, bool> ret = noDups.insert(std::make_pair(It->_host, It->_port));
		if(!ret.second)
			return(false);
	}
	return(true);
}

const std::vector<Server>& Configuration::getServers() {return(_Servers);}

std::ostream& operator<<(std::ostream &out, Configuration& c) {
	std::vector<Server>::iterator It;
	std::vector<Server> servers = c.getServers();
	out << "======================== CONFIGURATION ========================" << std::endl;
	for(It = servers.begin(); It != servers.end(); It++) {
		int serverCount = 0;
		out << "------------- SERVER : " << serverCount << " -------------" << std::endl;
		out << *It;
		serverCount++;
	}
	out << "===============================================================" << std::endl;
	return(out);
}