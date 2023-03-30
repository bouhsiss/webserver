#include "Server.hpp"

Server::Server() {};

Server::Server(const Server& other) {};

Server& Server::operator=(const Server& other) {};

Server::~Server() {};

bool ErrorMessage(std::string message) {
	std::cout << message << std::endl;
	return(EXIT_FAILURE);
}

bool Server::startServer() {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address;
	if(getaddrinfo(0, PORT, &hints, &bind_address) < 0)
		return(ErrorMessage("getaddrinfo() failed."));

	int socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if(socket_listen < 0)
		return(ErrorMessage("socket() failed"));
	if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) 
		return(ErrorMessage("bind() failed"));
	freeaddrinfo(bind_address);

	if(listen(socket_listen, 10) < 0)
		return(ErrorMessage("listen() failed"));
	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	int max_socket = socket_listen;
	while(1) {
		fd_set reads;
		reads = master;
		if(select(max_socket+1, &reads, 0, 0, 0) < 0)
			return(ErrorMessage("Select() failed"));
		int i;
		for(i = 1; i <= max_socket; ++i) {
			if(FD_ISSET(i, &reads)) {
				if(i == socket_listen) {
					struct sockaddr_in clienAddress;
					socklen_t clientLen = sizeof(clienAddress);
					int socketClient = accept(socket_listen, (struct sockaddr*) &clienAddress,  &clientLen);
					if(socketClient < 0)
						return(ErrorMessage("socket() failed"));
					FD_SET(socketClient, &master);
					if(socketClient > max_socket)
						max_socket = socketClient;
				}
				else {
					char read[1024];
					int bytes_received = recv(i, read, 1024, 0);
					if(bytes_received < 1) { 
						FD_CLR(i, &master);
						close(i);
						continue;
					}
				}
			}
		}
	}
	close(socket_listen);
}

