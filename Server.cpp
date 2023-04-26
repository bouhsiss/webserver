#include"Server.hpp"

const std::vector<Location>& Server::getLocations() {return(_Locations);}
const std::vector<std::string>& Server::getServerName() {return(_server_name);}
const std::string& Server::getHost() const {return(_host);}
const std::string& Server::getPort() const {return(_port);}
const std::vector<std::string>& Server::getErrorPage() {return(_error_page);}
const size_t& Server::getClientBodySizeLimit() const {return(_client_body_size_limit);}

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
	std::vector<std::string>::iterator It;
	std::vector<std::string> vect = c.getServerName();
	out << "      - server names : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	vect = c.getErrorPage();
	out << "      - error pages : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	std::vector<Location> locations = c.getLocations();
	std::vector<Location>::iterator Itr;
	out << "      - locations : " << std::endl;
	for(Itr = locations.begin(); Itr != locations.end(); Itr++)
		out << *Itr << std::endl;

	out << "=================================================================" << std::endl;
	return(out);
}