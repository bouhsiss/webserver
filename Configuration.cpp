#include "Configuration.hpp"

void trimSpaces(std::string &line) {
	line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
	line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
}

bool parseServerConfigLine(std::string line, Server& current_server) {
	int EqualSignPos = line.find('=');
	if(EqualSignPos == std::string::npos)
		return(false);
	std::string key = line.substr(0, EqualSignPos);
	std::string value = line.substr(EqualSignPos + 1);
	trimSpaces(key);
	trimSpaces(value);
	if(key == "listen")
		current_server.setListen(value);
	else if(key == "server_name")
		current_server.setServerName(value);
	else if(key == "host")
		current_server.setHost(value);
	else if(key == "erro_page")
		current_server.setErrorPage(value);
	else if(key == "client_body_size_limit")
		current_server.setClienBodySizeLimit(value);
	else
		return(false);
	return(true);
}

bool parseLocationConfigLine(std::string line, Location current_location) {
	int EqualSignPos = line.find('=');
	if(EqualSignPos == std::string::npos)
		return(false);
	std::string key = line.substr(0, EqualSignPos);
	std::string value = line.substr(EqualSignPos + 1);
	trimSpaces(key);
	trimSpaces(value);
	if(key == "path")
		current_location.setPath(value);
	else if (key == "allowed_methods")
		current_location.setAllowedMethods(value);
	else if(key == "root")
		current_location.setRoot(value);
	else if(key == "index")
		current_location.setIndex(value);
	else if(key == "redirect")
		current_location.setRedirect(value);
	else if(key == "autoindex")
		current_location.setIndex(value);
	else if(key == "default_file")
		current_location.setDefaultFile(value);
	else if(key == "upload_path")
		current_location.setUploadPath(value);
	else if(key == "cgi_extension")
		current_location.setCgiExtension(value);
	else if(key == "cgi_path")
		current_location.setCgiPath(value);
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

	if(!file.is_open()) {
		std::cout << "couldn't open file" << std::endl;
		return(EXIT_FAILURE);
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
		else if(inServer && line.find("location") != std::string::npos) {
			inLocation = true;
			currentLocation = Location();
			int pathStart = line.find_first_of("/");
			int pathEnd = line.find_last_of("{");
			currentLocation.setPath(line.substr(pathStart, pathEnd - pathStart));
			currentServer.addLocation(currentLocation);
		}
		else if(inLocation && braceCount == 0) {
			inLocation = false;
			currentServer.addLocation(currentLocation);
		}
		else if(inServer && braceCount == 0) {
			inServer = false;
			_Servers.push_back(currentServer);
		}
		else if(inServer && !inLocation) {
			if(!parseServerConfigLine(line, currentServer)) {
				std::cout << "invalid server block config line" << std::endl;
				return(EXIT_FAILURE);
			}
		}
		else if(inServer && inLocation) {
			if(!parseLocationConfigLine(line, currentLocation)) {
				std::cout << "invalid location block config line" << std::endl;
				return(EXIT_FAILURE);
			}
		}
	}
}