/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:41:43 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:41:46 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location() {
	_path = "";
	_redirect = "";
	_root = "";
	_autoindex = "";
	_index = "";
	_upload_path = "";
}

std::vector<std::string> Location::getAllowedMethods() {return(_allowed_methods);}
std::map<std::string, std::string> Location::getCgiPath() const {return(_cgi_path);}
std::string Location::getIndex() {return(_index);}
std::string Location::getPath() const {return(_path);}
std::string Location::getRoot() const {return(_root);}
std::string Location::getRedirect() const {return(_redirect);}
std::string Location::getAutoIndex() const {return(_autoindex);}
std::string Location::getUploadPath() const {return(_upload_path);}

void Location::setPath(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1)
		throw(Http::ConfigFileErrorException("Invalid location path"));
	this->_path = tokens[0];
	if(tokens[0][0] != '/')
		_path.insert(0,1,'/');
}

void Location::setAllowedMethods(std::vector<std::string> const &tokens) {
	if(tokens.size() == 0)
		throw(Http::ConfigFileErrorException("empty allowed_methods directive."));
	for(size_t i = 0; i < tokens.size(); i++) {
		if(tokens[i] == "GET" || tokens[i] == "POST" || tokens[i] == "DELETE") {
			if(std::find(_allowed_methods.begin(), _allowed_methods.end(), tokens[i]) != _allowed_methods.end())
				throw(Http::ConfigFileErrorException("duplicated value : " + tokens[0]));
			_allowed_methods.push_back(tokens[i]);
		}
		else
			throw(Http::ConfigFileErrorException("Invalid method : " + tokens[i]));
	}
}

void Location::setRedirect(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !_redirect.empty())
		throw(Http::ConfigFileErrorException("Invalid redirect directive"));
	this->_redirect = tokens[0];
}

void Location::setRoot(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !_root.empty() )
		throw(Http::ConfigFileErrorException("Invalid root directive"));
	this->_root = tokens[0];
}

void Location::setAutoIndex(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !_autoindex.empty() || (tokens[0] != "ON" && tokens[0] != "on" && tokens[0] != "off" && tokens[0] != "OFF" ))
		throw(Http::ConfigFileErrorException("Invalid autoindex directive"));
	this->_autoindex = tokens[0];
}

void Location::setIndex(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !_index.empty())
		throw(Http::ConfigFileErrorException("Invalid index directive"));
	this->_index = tokens[0];
}

void Location::setUploadPath(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !_upload_path.empty())
		throw(Http::ConfigFileErrorException("Invalid Upload path directive"));
	this->_upload_path = tokens[0];
}


void Location::setCgiPath(std::vector<std::string> const &tokens) {
	if(tokens.size() != 2 || _cgi_path.find(tokens[0]) != _cgi_path.end())
		throw(Http::ConfigFileErrorException("Invalid cgi path directive"));
	this->_cgi_path.insert(std::make_pair(tokens[0], tokens[1]));
}

void Location::isLocationComplete(Server &parentServer) {
	if(_path.empty())
		throw(Http::ConfigFileErrorException("Incomplete location configuration : path value missing."));
	if(_root.empty())
		_root = parentServer.getRoot();
	if(_index.empty())
		_index = parentServer.getIndex();
	if(_autoindex.empty())
		_autoindex = parentServer.getAutoIndex();
	if(std::find(_allowed_methods.begin(), _allowed_methods.end(), "GET") == _allowed_methods.end())
		_allowed_methods.push_back("GET");
	if(std::find(_allowed_methods.begin(), _allowed_methods.end(), "POST") == _allowed_methods.end())
		_allowed_methods.push_back("POST");
	if(std::find(_allowed_methods.begin(), _allowed_methods.end(), "POST") == _allowed_methods.end())
		_allowed_methods.push_back("POST");
}

std::ostream& operator<<(std::ostream &out, Location &c) {
	out << "======================== LOCATION BLOCK ========================" << std::endl;
	out << "      - location path : " << c.getPath() << std::endl;
	out << "      - location root : " << c.getRoot() << std::endl;
	out << "      - location redirect : " << c.getRedirect() << std::endl;
	out << "      - location autoindex : " << c.getAutoIndex() << std::endl;
	out << "      - location upload path : " << c.getUploadPath() << std::endl;
	out << "      - location cgi path : " << std::endl;
	std::map<std::string, std::string>::iterator mapIt;
	std::map<std::string, std::string> cgiPath = c.getCgiPath();
	for(mapIt = cgiPath.begin() ; mapIt != cgiPath.end() ; mapIt++)
	{
		std::cout << "key : " << mapIt->first << std::endl;
		std::cout << "value : " << mapIt->second << std::endl;
	}
	out << "      - location index : " << c.getIndex() << std::endl;
	std::vector<std::string> vect = c.getAllowedMethods();
	std::vector<std::string>::iterator It;
	out << "      - location allowed methods : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	out << "===============================================================" << std::endl;
	return(out);
}
