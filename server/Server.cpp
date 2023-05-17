/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassan <hassan@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:42:06 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/16 18:49:47 by hassan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include"Server.hpp"

std::map<std::string, Location *>& Server::getLocations() {return(_Locations);}
const std::string& Server::getServerName() {return(_server_name);}
const std::string& Server::getHost() const {return(_host);}
const std::string& Server::getPort() const {return(_port);}
const std::vector<std::string>& Server::getErrorPage() {return(_error_page);}
const size_t& Server::getClientBodySizeLimit() const {return(_client_body_size_limit);}
const int& Server::getListenSocket() const {return(_listenSocket);}

void Server::setPort(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !Http::strIsNumber(tokens[0]) || ( std::stoi(tokens[0]) < 1024 && std::stoi(tokens[0]) != 80 && std::stoi(tokens[0]) != 443 && std::stoi(tokens[0]) > 65535) || !this->_port.empty())
		throw(Http::ConfigFileErrorException("Invalid port directive"));
	this->_port = tokens[0];
}

void Server::setServerName(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1  || !_server_name.empty())
		throw(Http::ConfigFileErrorException("Invalid server name directive"));
	this->_server_name = tokens[0];
}

void Server::setHost(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !this->_host.empty())
		throw(Http::ConfigFileErrorException("invalid host directive"));
	this->_host = tokens[0];
}

void Server::setErrorPage(std::vector<std::string> const &tokens) {
	if(tokens.empty())
		throw(Http::ConfigFileErrorException("invalid error_page directive"));
	if(std::find(_error_page.begin(), _error_page.end(), tokens[0]) != _error_page.end())
		throw(Http::ConfigFileErrorException("duplicated value : " + tokens[0]));
	this->_error_page = tokens;
}

void Server::setClientBodySizeLimit(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !Http::strIsNumber(tokens[0]) || std::stoi(tokens[0]) < 0 || this->_client_body_size_limit != (size_t)-1)
		throw(Http::ConfigFileErrorException("Invalid client body size limit directive"));
	this->_client_body_size_limit = std::stoi(tokens[0]);
}


Server::Server() {
	this->_port = "";
	this->_host = "";
	this->_client_body_size_limit = -1;
	this->_server_name = "";
}

Server::~Server() {
	std::map<std::string, Location *>::iterator It;
	for(It = _Locations.begin(); It != _Locations.end(); It++)
		delete It->second;
}

void Server::setServerDefaultValues() {
	if(_host.empty())
		_host = "127.0.0.1";
	if(_port.empty())
		_port = 80;
	if(_client_body_size_limit == (size_t)-1)
		_client_body_size_limit = 100000;
	std::map<std::string, Location *>::iterator It;
	for(It = _Locations.begin(); It != _Locations.end(); It++)
		It->second->isLocationValid();
}

void Server::setupListenSocket() {
	struct addrinfo hints, *res;
	std::cout << GREEN << "Configuring local address..." << RESET << std::endl;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int status = getaddrinfo(_host.c_str(), _port.c_str(), &hints, &res);
	if(status)
		throw(Http::NetworkingErrorException("getaddrinfo error :" + std::string(gai_strerror(status))));
	std::cout << GREEN << "Creating listening socket.." << RESET << std::endl;
	_listenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(_listenSocket < 0)
		throw(Http::NetworkingErrorException("socket() failed"));
	// avoid the bind failure (address already in use)
	int yes = 1;
	if(setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		throw(Http::NetworkingErrorException("setsockopt() failed.."));
	std::cout << GREEN << "Binding socket to local address.. " << RESET << std::endl;
	if(bind(_listenSocket, res->ai_addr, res->ai_addrlen) < 0)
		throw(Http::NetworkingErrorException("bind() to : " + _host + "  failed"));
	std::cout << GREEN << "Listening on " << RESET << std::endl;
	Http::printAddr(res);
	std::cout << YELLOW << "Remote port is : " << _port << RESET << std::endl;
	freeaddrinfo(res);
	if(listen(_listenSocket, BACKLOG) < 0)
		throw(Http::NetworkingErrorException("listen() failed") );
}

std::ostream& operator<<(std::ostream &out, Server &c) {
	out << "======================== SERVER BLOCK ========================" << std::endl;
	out << "      - server host : " << c.getHost() << std::endl;
	out << "      - server port : " << c.getPort() << std::endl;
	out << "      - server client body size limit : " << c.getClientBodySizeLimit() << std::endl;
	out << "      - server server name : " << c.getServerName() << std::endl;
	std::vector<std::string>::iterator It;
	std::vector<std::string> vect = c.getErrorPage();;
	out << "      - error pages : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	std::map<std::string, Location *> locations = c.getLocations();
	std::map<std::string, Location *>::iterator Itr;
	out << "      - locations : " << std::endl;
	for(Itr = locations.begin(); Itr != locations.end(); Itr++)
	{
		out << *Itr->second << std::endl;
	}


	out << "=================================================================" << std::endl;
	return(out);
}
