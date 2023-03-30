#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#include <fstream>

#define PORT 80

class Server {
	public :
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();
		bool startServer();
		const std::string& getRequest();
	private :
		std::string RequestData;
};