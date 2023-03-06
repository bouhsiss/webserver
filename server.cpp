#include "socket.hpp"


//server side

int main(int ac, char ** av) {

	struct sockaddr_in address; // transport address structure
	memset(address.sin_zero, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY is an IP add used when we don't want to bind a socket to any specific IP, when we don't know the IP add of our machine, we can use the special IP add INADDR_ANY
	address.sin_port = htons(PORT);

	// creatin a socket with a socket() system call
	int serverSocket = socket(address.sin_family, SOCK_STREAM, 0);
	if(serverSocket < 0)
		std::cout << "cannot create server socket." << std::endl;
	
	// naming the socket - assigning the transpot address to the socket 
	if(bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
		std::cout << "bind failed" << std::endl;

	// prepare the original socket to only accept the connections (can't be used to exchange data)
	if(listen(serverSocket, 4) < 0) {
		std::cout << "listen Failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// the accept system call grabs the first connection request on the queue and creates a new socket for that connection
	int new_socket;
	int addrelen = sizeof(address);
	while(1) {
		std::cout << "<--------------Waiting for new connections -------------->" << std::endl;
		if((new_socket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrelen)) < 0) {
			std::cout << "accept Failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		// send and receive messages
		char buffer[1024] = {0};
		int readEnd = read(new_socket, buffer, 1024);
		std::cout << buffer << std::endl;
		if(readEnd < 0)
			std::cout << "No bytes to read" << std::endl;
		
		std::string hello = "hello from the server";
		write(new_socket, hello.c_str(), hello.size());
		std::cout << "<--------------Hello message sent from server--------------->" << std::endl;
		close(new_socket);
	}
}