#pragma once 
#include <iostream>

class Location {
    public :
        std::string path;
        std::string allow_methods;
        std::string root;
        std::string index;
        std::string redirect;
        std::string autoindex;
        std::string default_file;
        std::string upload_path;
        std::string body_size_limit;
        std::string cgi_extension;
        std::string cgi_path;
};