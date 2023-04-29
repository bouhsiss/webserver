#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>
#include "Location.hpp"
//addrinfo
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
//select()
#include <sys/select.h>



class Server {
    public :
		const std::vector<Location>& getLocations();
		const std::string& getServerName();
		const std::string& Server::getHost() const;
		const int& Server::getPort() const;
		const std::vector<std::string>& getErrorPage();
		const size_t& getClientBodySizeLimit() const;
		
		void setPort(std::vector<std::string> &tokens);
		void setServerName(std::vector<std::string> &tokens);
		void setHost(std::vector<std::string> &tokens);
		void setErrorPage(std::vector<std::string> &tokens);
		void setClientBodySizeLimit(std::vector<std::string> &tokens);
		

		Server();

		void run();

    private :
		int listenSocket;

        std::vector<Location> _Locations;    
        std::string _server_name;
		std::string _host;
		int _port;
        std::vector<std::string> _error_page;
        size_t _client_body_size_limit;
		friend  class Configuration;
};

//notes to keep in mind
// if the port is not given the port 80 is assumed 
// an empty abs_path is equivalent to "/"

std::ostream& operator<<(std::ostream &out, Server& c);