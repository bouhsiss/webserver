#pragma once
#include <iostream>
#include "Location.hpp"

class Server {
    public :
        std::vector<Location> _Locations;    
        std::string listen;
        std::string server_name;
        std::string host;
        std::string error_page;
        std::string client_body_size_limit;
};