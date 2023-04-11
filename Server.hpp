#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "Location.hpp"

class Server {
    public :

    private :
        std::vector<Location> _Locations;    
        std::vector<std::string> _server_name;
        std::string _host;
        std::string _port;
        std::vector<std::string> _error_page;
        size_t _client_body_size_limit;
		friend  class Configuration;
};

//gotta implement an overload for << operator