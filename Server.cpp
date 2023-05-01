#include"Server.hpp"

const std::map<std::string, Location *>& Server::getLocations() {return(_Locations);}
const std::string& Server::getServerName() {return(_server_name);}
const std::string& Server::getHost() const {return(_host);}
const int& Server::getPort() const {return(_port);}
const std::vector<std::string>& Server::getErrorPage() {return(_error_page);}
const int& Server::getClientBodySizeLimit() const {return(_client_body_size_limit);}

void Server::setPort(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !Http::strIsNumber(tokens[0]) || std::stoi(tokens[0]) < 0 || std::stoi(tokens[0]) > 1023 || this->_port != -1)
		throw(Http::ConfigFileErrorException("Invalid port directive"));
	this->_port = std::stoi(tokens[0]);
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
	if(tokens.size() != 1 || !Http::strIsNumber(tokens[0]) || std::stoi(tokens[0]) < 0 || this->_client_body_size_limit != -1)
		throw(Http::ConfigFileErrorException("Invalid client body size limit directive"));
	this->_client_body_size_limit = std::stoi(tokens[0]);
}


Server::Server() {
	// struct addrinfo hints;
	// memset(&hints, 0, sizeof(hints));
	// hints.ai_family = AF_INET;
	// hints.ai_socktype = SOCK_STREAM;
	// hints.ai_flags = AI_PASSIVE;

	// struct addrinfo *bind_address;
	// getaddrinfo(this->_host.c_str(), this->_port.c_str(), &hints, &bind_address);

	// std::cout << GREEN << "Creating sockets." << RESET << std::endl;
	// listenSocket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	// if(listenSocket < 0) {
	// 	std::cout << RED << "socket() failed " << RESET << std::endl;
	// 	return;
	// }
	// if(bind(listenSocket, bind_address->ai_addr, bind_address->ai_addrlen)) {
	// 	std::cout << RED << "bind() failed." << RESET << std::endl;
	// 	return; 
	// }
	// freeaddrinfo(bind_address);
	// std::cout << GREEN  << "server listening on " << this->_host << "::" << this->_port << RESET << std::endl;
	// if(listen(listenSocket, 10) < 0) {
	// 	std::cout << RED << "listen() failed.." << RESET << std::endl;
	// 	return;
	// }
	this->_port = -1;
	this->_host = "";
	this->_client_body_size_limit = -1;
	this->_server_name = "";
}

void Server::run() {
	fd_set master; // an fd set to store all active sockets
	FD_ZERO(&master);
	FD_SET(listenSocket, &master);
	int maxSocket = listenSocket;
	while(true) {
		fd_set reads;
		reads = master;
		if(select(maxSocket+1, &reads, 0, 0, 0) < 0) {
			std::cout << RED << "select() failed " << RESET << std::endl;
			return;
		}
		// check on all sockets to check whether a socket was flagged by select
		int i;
		for(i = 1; i <= maxSocket; i++) {
			if(FD_ISSET(i, &reads)) {
				if(i == listenSocket) {
					//new client has connected
					struct sockaddr_in clientAddress; 
					socklen_t clientLen = sizeof(clientAddress);
					int clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientLen);
					if(clientSocket < 0) {
						std::cout << RED << "accept() failed" << RESET << std::endl;
						return;
					}
					FD_SET(clientSocket, &master);
					if(clientSocket > maxSocket)
						maxSocket = clientSocket;
					char address_buffer[100];
					getnameinfo((struct sockaddr*)&clientAddress,
                            clientLen,
                            address_buffer, sizeof(address_buffer), 0, 0,
                            NI_NUMERICHOST);
					std::cout << BLUE << "new connection\n from " << RESET << address_buffer << std::endl;
				}
				else {
					//client has sent request
					char read[1024];
					int bytes_received = recv(i, read, 1024, 0);
					if(bytes_received < 1) // if client has disconnected then recv() returns a non positive number in this case we remove the socket from the master socket set
					{ 
						FD_CLR(i, &master);
						close(i);
						continue;
					}
					std::cout << BLUE << "Request :"  << RESET << std::endl << std::endl;
					std::string str(read , bytes_received);
					std::cout << str << std::endl;
					std::cout << "=================" << std::endl;
					send(i, read, bytes_received, 0);
				}
			}
		}
	}
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
		out << *Itr->second << std::endl;

	out << "=================================================================" << std::endl;
	return(out);
}