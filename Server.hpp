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
		const std::map<std::string, Location *>& getLocations();
		const std::string& getServerName();
		const std::string& getHost() const;
		const int& getPort() const;
		const std::vector<std::string>& getErrorPage();
		const int& getClientBodySizeLimit() const;
		
		void setPort(std::vector<std::string> const &tokens);
		void setServerName(std::vector<std::string> const &tokens);
		void setHost(std::vector<std::string> const &tokens);
		void setErrorPage(std::vector<std::string> const &tokens);
		void setClientBodySizeLimit(std::vector<std::string> const &tokens);
		

		Server();

		void run();

    private :
		int listenSocket;

		std::map<std::string, Location *> _Locations;
        std::string _server_name;
		std::string _host;
		int _port;
        std::vector<std::string> _error_page;
        int _client_body_size_limit;
		friend  class Configuration;
};

//notes to keep in mind
// if the port is not given the port 80 is assumed 
// an empty abs_path is equivalent to "/"

std::ostream& operator<<(std::ostream &out, Server& c);