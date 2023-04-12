#pragma once 
#include <iostream>
#include <vector>
class Location {
    public :
		const std::vector<std::string> getAllowedMethods();
		const std::vector<std::string> getIndex();
		const std::string getPath() const;
		const std::string getRoot() const;
		const std::string getRedirect() const;
		const std::string getAutoIndex() const;
		const std::string getUploadPath() const;
		const std::string getCgiExtension() const;
		const std::string getCgiPath() const;
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

std::ostream& operator<<(std::ostream &out, Location &c);