#pragma once 
#include <iostream>

class Location {
    public :
        //***===  getters  ===***
        const std::string getPath();
        const std::string getAllowedMethods();
        const std::string getRoot();
        const std::string getIndex();
        const std::string getRedirect();
        const std::string getAutoIndex();
        const std::string getDefaultFile();
        const std::string getUploadPath();
        const std::string getCgiExtension();
        const std::string getCgiPath();

        //***===  setters  ===***
        void setPath(std::string path);
        void setAllowedMethods(std::string allowedMethods);
        void setRoot(std::string root);
        void setIndex(std::string index);
        void setRedirect(std::string redirect);
        void setAutoIndex(std::string autoIndex);
        void setDefaultFile(std::string defaultFile);
        void setUploadPath(std::string uploadPath);
        void setCgiExtension(std::string cgiExtension);
        void setCgiPath(std::string cgiPath);

    private :
        std::string _path;
        std::string _allowed_methods;
        std::string _root;
        std::string _index;
        std::string _redirect;
        std::string _autoindex;
        std::string _default_file;
        std::string _upload_path;
        std::string _cgi_extension;
        std::string _cgi_path;
};