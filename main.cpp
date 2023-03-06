#include "socket.hpp"

int main(int ac, char ** av) {
	// creatin a socket with a socket() system call
	int serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	if(serverSocket < 0)
		std::cout << "cannot create socket." << std::endl;

	// naming the socket - assigning a transpot address to the socket (a port nuumber) 
		/* this transport address is defined in a socket address structure */
	struct sockaddr_in address;
	
}