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
		const std::string& getPort() const;
		const std::vector<std::string>& getErrorPage();
		const int& getClientBodySizeLimit() const;
		
		void setPort(std::vector<std::string> const &tokens);
		void setServerName(std::vector<std::string> const &tokens);
		void setHost(std::vector<std::string> const &tokens);
		void setErrorPage(std::vector<std::string> const &tokens);
		void setClientBodySizeLimit(std::vector<std::string> const &tokens);

		void isServerValid();
		void startListening();

		Server();


    private :
		int listenSocket;

		std::map<std::string, Location *> _Locations;
        std::string _server_name;
		std::string _host;
		std::string _port;
        std::vector<std::string> _error_page;
        int _client_body_size_limit;
		friend  class Configuration;
};

//notes to keep in mind
// if the port is not given the port 80 is assumed 
// an empty abs_path is equivalent to "/"

std::ostream& operator<<(std::ostream &out, Server& c);

/*
code to convert a address struct to printable string 
char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
struct sockaddr_in sa;      // pretend this is loaded with something

inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN); // network to presentation

printf("The IPv4 address is: %s\n", ip4);


// IPv6:

char ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
struct sockaddr_in6 sa6;    // pretend this is loaded with something

inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);

printf("The address is: %s\n", ip6);
*/