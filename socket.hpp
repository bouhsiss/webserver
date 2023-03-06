#pragma once

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


#define PORT 8080 

// //Internet address (in_addr is more than just an integer because it might have more than s_addr depending on the OS, in some systems it has union)
// struct in_addr {
// 	uint32_t s_addr; // holds t
// };

// // the address structure definition
// struct sockaddr_in {
// 	short int sin_family; // address family
// 	unsigned short int sin_port; // port number
// 	struct in_addr sin_addr; // internet address
// 	unsigned char sin_zero[8]; // this is included to pad the structure to the length of the struct sockaddr
// };