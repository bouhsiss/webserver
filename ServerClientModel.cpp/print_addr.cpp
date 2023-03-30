#include <string.h>  
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   
#include <arpa/inet.h>    
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <netdb.h>

int main (int ac , char **av) {
	if(ac != 2) {
		std::cout << "Usage : <./print_addr> hostname" << std::endl;
		return(EXIT_FAILURE);
	}
	std::string arg(av[1]);
	std::cout << "resolving hostname" << arg << std::endl;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_ALL;
	struct addrinfo *peer_address;
	if(getaddrinfo(av[1] , 0, &hints, &peer_address)) {
		std::cout << "getaddrinfo() failed" << std::endl;
		return(EXIT_FAILURE);
	}
	std::cout << "Remote address is :" ;
	struct addrinfo *address = peer_address;
	char address_buffer[100];
	getnameinfo(address->ai_addr, address->ai_addrlen, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
	std::cout << address_buffer << std::endl;
}