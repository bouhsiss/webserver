#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "Location.hpp"


class Server {
    public :
		const std::vector<Location>& getLocations();
		const std::vector<std::string>& getServerName();
		const std::string& getHost() const;
		const std::string& getPort() const;
		const std::vector<std::string>& getErrorPage();
		const size_t& getClientBodySizeLimit() const;

    private :
        std::vector<Location> _Locations;    
        std::vector<std::string> _server_name;
        std::string _host;
        std::string _port;
        std::vector<std::string> _error_page;
        size_t _client_body_size_limit;
		friend  class Configuration;
};

//notes to keep in mind
// if the port is not given the port 80 is assumed 
// an empty abs_path is equivalent to "/"

std::ostream& operator<<(std::ostream &out, Server& c);