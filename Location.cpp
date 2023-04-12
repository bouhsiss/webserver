#include "Location.hpp"

const std::vector<std::string> Location::getAllowedMethods() {return(_allowed_methods);}
const std::vector<std::string> Location::getIndex() {return(_index);}
const std::string Location::getPath() const {return(_path);}
const std::string Location::getRoot() const {return(_root);}
const std::string Location::getRedirect() const {return(_redirect);}
const std::string Location::getAutoIndex() const {return(_autoindex);}
const std::string Location::getUploadPath() const {return(_upload_path);}
const std::string Location::getCgiExtension() const {return(_cgi_extension);}
const std::string Location::getCgiPath() const {return(_cgi_path);}



std::ostream& operator<<(std::ostream &out, Location &c) {
	out << "======================== LOCATION BLOCK ========================" << std::endl;
	out << "      - location path : " << c.getPath() << std::endl;
	out << "      - location root : " << c.getRoot() << std::endl;
	out << "      - location redirect : " << c.getRedirect() << std::endl;
	out << "      - location autoindex : " << c.getAutoIndex() << std::endl;
	out << "      - location upload path : " << c.getUploadPath() << std::endl;
	out << "      - location cgi extension : " << c.getCgiExtension() << std::endl;
	out << "      - location cgi path : " << c.getCgiPath() << std::endl;
	std::vector<std::string> vect = c.getAllowedMethods();
	std::vector<std::string>::iterator It;
	out << "      - location allowed methods : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	vect = c.getIndex();
	out << "      - location indexes : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	out << "===============================================================" << std::endl;
	return(out);
}