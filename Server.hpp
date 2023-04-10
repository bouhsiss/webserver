#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "Location.hpp"

class Server {
    public :
        //***===  setters  ===***
        void addLocation(Location Location);
        void setListen(std::string listen);
        void setServerName(std::string serverName);
        void setHost(std::string host);
        void setErrorPage(std::string errorPage);
        void setClienBodySizeLimit(std::string clientBodySizeLimit);

        //***===  getters  ===***
        const std::vector<Location>& getLocations();
        const Location& getLocation(std::string path);
        const std::string getListen();
        const std::string getServerNAme();
        const std::string getHost();
        const std::string getErrorPage();
        const std::string getClientBodySizeLimit();
    private :
        std::vector<Location> _Locations;    
        std::string _listen;
        std::string _server_name;
        std::string _host;
        std::string _error_page;
        std::string _client_body_size_limit;
};

//gotta implement an overload for << operator