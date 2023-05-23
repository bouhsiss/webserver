/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configuration.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:40:27 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:40:30 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Configuration.hpp"

bool Configuration::_extractServerConfigLine(std::string line, Server& current_server) {
	size_t EqualSignPos = line.find('=');
	if(EqualSignPos == std::string::npos)
		return(false);
	std::string key = line.substr(0, EqualSignPos);
	std::string value = line.substr(EqualSignPos + 1);
	Http::trimSpaces(key);
	Http::trimSpaces(value);
	

	if(key == "port")
		current_server.setPort(Http::tokenize(value, " "));
	else if(key == "server_name")
		current_server.setServerName(Http::tokenize(value, " "));
	else if(key == "host")
		current_server.setHost(Http::tokenize(value, " "));
	else if(key == "error_page")
		current_server.setErrorPage(Http::tokenize(value, " "));
	else if(key == "client_body_size_limit")
		current_server.setClientBodySizeLimit(Http::tokenize(value, " "));
	else if(key == "root")
		current_server.setRoot(Http::tokenize(value, " "));
	else if(key == "index")
		current_server.setIndex(Http::tokenize(value, " "));
	else if(key == "autoindex")
		current_server.setAutoIndex(Http::tokenize(value, " "));
	else
		return(false);
	return(true);
}

bool Configuration::_extractLocationConfigLine(std::string line, Location& current_location) {
	size_t EqualSignPos = line.find('=');
	if(EqualSignPos == std::string::npos)
		return(false);
	std::string key = line.substr(0, EqualSignPos);
	std::string value = line.substr(EqualSignPos + 1);
	Http::trimSpaces(key);
	Http::trimSpaces(value);

	if(key == "allowed_methods")
		current_location.setAllowedMethods(Http::tokenize(value, " "));
	else if(key == "redirect")
		current_location.setRedirect(Http::tokenize(value, " "));
	else if(key == "root")
		current_location.setRoot(Http::tokenize(value, " "));
	else if(key == "autoindex")
		current_location.setAutoIndex(Http::tokenize(value, " "));
	else if(key == "index")
		current_location.setIndex(Http::tokenize(value, " "));
	else if(key == "upload_path")
		current_location.setUploadPath(Http::tokenize(value, " "));
	else if(key == "cgi_extension")
		current_location.setCgiExtension(Http::tokenize(value, " "));
	else if(key == "cgi_path")
		current_location.setCgiPath(Http::tokenize(value, " "));
	else
		return(false);
	return(true);
}

std::vector<Server *> Configuration::parse(std::string configFilePath) {
	std::string conf(configFilePath);
	std::ifstream file(conf);
	std::string line;

	bool inServer = false;
	bool inLocation = false;

	Server *currentServer;
	Location *currentLocation;

	std::vector<Server *> servers;

	int openedBraces = 0;
	int closedBraces = 0;
	int braceCount = 0;

	if(!file.is_open()) {
		throw(Http::ConfigFileErrorException("Couldn't open file."));
	}
	while(std::getline(file, line)) {
		//strip comments if found
		size_t pos = line.find("#");
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
			currentServer = new Server();
		}
		else if(line.find("location ") != std::string::npos) {

			/*---------- check if the location block is valid*/
			if(braceCount != 2) {
				throw(Http::ConfigFileErrorException("missing curly brace in location block"));
			}
			if(inLocation == true || inServer == false) {
				throw(Http::ConfigFileErrorException("location block Invalid inheritance"));
			}
			inLocation = true;
			currentLocation = new Location();
			int pathStart = 9;
			int pathEnd = line.find_last_of("{");
			currentLocation->setPath(Http::tokenize(line.substr(pathStart, pathEnd - pathStart), " "));
			if(currentServer->_Locations.find(currentLocation->getPath()) != currentServer->_Locations.end())
				throw(Http::ConfigFileErrorException("Location path duplicated"));
		}
		else if(inLocation && braceCount == 1) {
			inLocation = false;
			currentServer->_Locations.insert(std::make_pair(currentLocation->getPath(), currentLocation));
		}
		else if(inServer && braceCount == 0) {
			inServer = false;
			currentServer->setServerDefaultValues();
			servers.push_back(currentServer);
		}
		else if(inServer && !inLocation) {
			if(!_extractServerConfigLine(line, *currentServer)) {
				throw(Http::ConfigFileErrorException("invalid server block config line : " + line));
			}
		}
		else if(inLocation) {
			if(!_extractLocationConfigLine(line, *currentLocation)) {
				throw(Http::ConfigFileErrorException("invalid location block config line : " + line));
			}
		}
		else {
			throw(Http::ConfigFileErrorException("invalid syntax"));
		}
	}
	return servers;
}
