#include"socket.hpp"

void exchangeData(int socketClient) {
	// send and receive messages
	char buffer[1024] = {0};
	int bytesReceived = recv(socketClient, buffer, 1024, 0);
	write(1, buffer, bytesReceived);
	if(bytesReceived <= 0)
		std::cout << "No bytes to read" << std::endl;
	
	std::string hello = "hello from the server";
	int bytesSent = send(socketClient, hello.c_str(), hello.size(), 0);
	std::cout << "Sent " << bytesSent << "of " << hello.size() << std::endl;
	std::cout << "<--------------Hello message sent from server--------------->" << std::endl;
}

int main() {
	std::cout << "configuring local address..." << std::endl;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // indicates that the server can bind to all available network interfaces and the operating system will choose the appropriate network interface to use based on the incoming connection

	struct addrinfo *bindAddress;
	if(getaddrinfo(NULL, PORT, &hints, &bindAddress) < 0) // we use the getaddrinfo to fill in the addrinfo struct with the needed information.  <----- this function has many uses but for our purpose it generate an address that is suitable for bind() and to make it generate this we must pass in the first prameter as NULL and have the AI_PASSIVE flag set in the hints.ai_flags ------->
	{
		std::cout << "getaddrinfo failed" << std::endl;
		return(EXIT_FAILURE);
	}
	std::cout << "Creating socket...." << std::endl;
	int socketListen;
	socketListen = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol);
	if(socketListen < 0) {
		std::cout << "socket() failed" << std::endl;
		return(EXIT_FAILURE);
	}

	std::cout << "binding socket to local address.." << std::endl;
	if(bind(socketListen, bindAddress->ai_addr, bindAddress->ai_addrlen)) {
		std::cout << "bind() failed" << std::endl;
		return(EXIT_FAILURE);
	}
	freeaddrinfo(bindAddress); //after we have bound to bindAddress we can release the address memory

	std::cout << "Listening for connections" << std::endl;
	if(listen(socketListen, 10) < 0 ) { // the second parameter tells listen() how many connections it's allowed to queue up - if 10 connection become queued up the the OS will reject new connections until we remove one from the existing queue 
		std::cout << "listen failed" << std::endl;
		return(EXIT_FAILURE);
	}

	struct sockaddr_in clientAddress; // store the address info for the connecting client
	int socketClient;
	int clientLen = sizeof(clientAddress);
	while(1) {
		std::cout << "<--------------Waiting for new connections -------------->" << std::endl;
		/* accept has a few functions, first - it will block your program until a new connection is made. in other words, your program will sleep until a connection is made to the listening socket. when the connection is made accept() will create a new socket for it. it also fills in address info of the client connected */
		if((socketClient = accept(socketListen, (struct sockaddr*)&clientAddress , (socklen_t*)&clientLen)) < 0) {
			std::cout << "accept Failed" << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "<----------- Client is connected ---------->" << std::endl;
		exchangeData(socketClient);
		std::cout << "Closing connection.. " << std::endl; // if we don't close the connection the client will wait for more data until it times out.
		close(socketClient);
	}
}