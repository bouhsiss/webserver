#pragma once 
#include <iostream>
#include <vector>
class Location {
    public :

    private :
        std::string _path;
        std::vector<std::string> _allowed_methods;
        std::string _root;
        std::vector<std::string> _index;
        std::string _redirect;
        std::string _autoindex;
        std::string _upload_path;
        std::string _cgi_extension;
        std::string _cgi_path;

		friend class Configuration;
};