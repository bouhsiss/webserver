#include "socket.hpp"

int main() {
	std::cout << "configuring local address... " << std::endl;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address;
	getaddrinfo(0, "8282", &hints, &bind_address);

	std::cout << "Creating socket.." << std::endl;
	int socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if(socket_listen < 0) {
		std::cout << "socket() failed " << std::endl;
		return(EXIT_FAILURE);
	}
	std::cout << "binding socket to local address.." << std::endl;
	if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
		std::cout << "bind() failed" << std::endl;
		return(EXIT_FAILURE);
	}
	freeaddrinfo(bind_address);

	std::cout << "Listening.." << std::endl;
	if(listen(socket_listen, 10) < 0) {
		std::cout << "listen() failed.." << std::endl;
		return(EXIT_FAILURE);
	}
	// we define an fd_set to store all active sockets
	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	int max_socket = socket_listen;
	std::cout << "waiting for connections.." << std::endl;
	while(1) {
		fd_set reads;
		reads = master;
		if(select(max_socket+1, &reads, 0, 0, 0) < 0) {
			std::cout << "select() failed " << std::endl;
			return(EXIT_FAILURE);
		}
		// check on all sockets to check whether a socket was flagged by select
		int i;
		for(i = 1; i <= max_socket; ++i) {
			if(FD_ISSET(i, &reads)) {
				// check whether the socket is the listening one or not 
				if(i == socket_listen) {
					struct sockaddr_in clienAddress;
					socklen_t clientLen = sizeof(clienAddress);
					int socketClient = accept(socket_listen, (struct sockaddr*) &clienAddress,  &clientLen);
					if(socketClient < 0) {
						std::cout << "accept() failed" << std::endl;
						return(EXIT_FAILURE);
					}
					FD_SET(socketClient, &master);
					if(socketClient > max_socket)
						max_socket = socketClient;
					char address_buffer[100];
                    getnameinfo((struct sockaddr*)&clienAddress,
                            clientLen,
                            address_buffer, sizeof(address_buffer), 0, 0,
                            NI_NUMERICHOST);
					std::cout << "new connection\n from " << address_buffer << std::endl;
				}
				else {
					char read[1024];
					int bytes_received = recv(i, read, 1024, 0);
					if(bytes_received < 1) // if client has disconnected then recv() returns a non positive number in this case we remove the socket from the master socket set
					{ 
						FD_CLR(i, &master);
						close(i);
						continue;
					}
					int j;
					for (j = 0; j < bytes_received; ++j)
							read[j] = toupper(read[j]);
					send(i, read, bytes_received, 0);
				}
			}
		}
	}
	std::cout << "closing listening socket.." << std::endl;
	close(socket_listen);
	std::cout << "finished" << std::endl;
}