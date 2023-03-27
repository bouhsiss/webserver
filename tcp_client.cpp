#include "socket.hpp"

int main() {
	std::cout << "configuring remote address.." << std::endl;
	struct addrinfo hints;
	memset(&hints, 0,sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	if(getaddrinfo("127.0.0.1", "8282", &hints, &peer_address)) {
		std::cout << "get addrinfo() failed" << std::endl;
		return(EXIT_FAILURE);
	}

	printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
            address_buffer, sizeof(address_buffer),
            service_buffer, sizeof(service_buffer),
            NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

	std::cout << "creating the socket.." << std::endl;
	int socket_peer;
	socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	if(socket_peer < 0) {
		std::cout << "socket() failed..." << std::endl;
		return(EXIT_FAILURE);
	}
	std::cout << "Connecting.. " << std::endl;
	if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
		std::cout << "connect() failed ... " << std::endl;
		return(EXIT_FAILURE);
	}
	freeaddrinfo(peer_address);
	std::cout << "connected.." << std::endl;
	std::cout << "to send data, enter text followed by enter" << std::endl;
	// now our program should loop and check the terminal and the socket for new data, if new data comes from the terminal it sends it over the socket and if data is read from the socket it's printed to the terminal
	while(1) {
		fd_set  reads;
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads);
		FD_SET(0, &reads); // only on non windows systems

		struct timeval timeout; // a time limit for select
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		if(select(socket_peer + 1, &reads, 0, 0, NULL) < 0) {
			std::cout << "select() failed." << std::endl;
			return(EXIT_FAILURE);
		}
		if(FD_ISSET(socket_peer, &reads)) {
			char read[4096];
 			int bytes_received = recv(socket_peer, read, 4096, 0);
			if(bytes_received < 1) {
				std::cout << "connection closed by peer " << std::endl;
				break ;
			}
			std::string substr = std::string(read).substr(0, bytes_received);
			std::cout << "Received " << bytes_received <<  " == " << substr <<  std::endl;
		}
		if(FD_ISSET(0, &reads)) {
			char read[4096];
 			if (!fgets(read, 4096, stdin)) break;
			std::cout << "sending " << read << std::endl;
			int bytes_sent = send(socket_peer, read, strlen(read), 0);
			std::cout << "sent "  << bytes_sent << " bytes" << std::endl;
		}
	}
	std::cout << "closing socket.." << std::endl;
	close(socket_peer);
	std::cout << "Finished." << std::endl;
	return(EXIT_SUCCESS);
}