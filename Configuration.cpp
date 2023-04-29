#include "Configuration.hpp"
#include "Exception.hpp"

bool Configuration::_extractServerConfigLine(std::string line, Server& current_server) {
	int EqualSignPos = line.find('=');
	if(EqualSignPos == std::string::npos)
		return(false);
	std::string key = line.substr(0, EqualSignPos);
	std::string value = line.substr(EqualSignPos + 1);
	Http::trimSpaces(key);
	Http::trimSpaces(value);
	std::vector<std::string> tokens;
	
	// if(key == "port")
	// 	current_server._port = value;
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

std::vector<Server> Configuration::parse(std::string configFilePath) {
	std::string conf(configFilePath);
	std::ifstream file(conf);
	std::string line;

	bool inServer = false;
	bool inLocation = false;

	Server currentServer;
	Location currentLocation;

	std::vector<Server> servers;

	int openedBraces = 0;
	int closedBraces = 0;
	int braceCount = 0;

	if(!file.is_open()) {
		throw(Http::ConfigFileErrorException("Couldn't open file."));
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
				throw(Http::ConfigFileErrorException("missing curly brace in server block"));
			}
			if(inServer || inLocation) {
				throw(Http::ConfigFileErrorException("server block Invalid inheritance"));
			}
			inServer = true;
			currentServer = Server();
		}
		else if(line.find("location /") != std::string::npos) {
			/*---------- check if the location block is valid*/
			if(braceCount != 2) {
				throw(Http::ConfigFileErrorException("missing curly brace in location block"));
			}
			if(inLocation == true || inServer == false) {
				throw(Http::ConfigFileErrorException("location block Invalid inheritance"));
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
			servers.push_back(currentServer);
		}
		else if(inServer && !inLocation) {
			if(!_extractServerConfigLine(line, currentServer)) {
				throw(Http::ConfigFileErrorException("invalid server block config line"));
			}
		}
		else if(inLocation) {
			if(!_extractLocationConfigLine(line, currentLocation)) {
				throw(Http::ConfigFileErrorException("invalid location block config line"));
			}
		}
		else {
			throw(Http::ConfigFileErrorException("invalid syntax"));
		}
	}
	return servers;
}
