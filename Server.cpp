#include"Server.hpp"

const std::vector<Location>& Server::getLocations() {return(_Locations);}
const std::vector<std::string>& Server::getServerName() {return(_server_name);}
const std::string& Server::getHost() const {return(_host);}
const std::string& Server::getPort() const {return(_port);}
const std::vector<std::string>& Server::getErrorPage() {return(_error_page);}
const size_t& Server::getClientBodySizeLimit() const {return(_client_body_size_limit);}


std::ostream& operator<<(std::ostream &out, Server &c) {
	out << "======================== SERVER BLOCK ========================" << std::endl;
	out << "      - server host : " << c.getHost() << std::endl;
	out << "      - server port : " << c.getPort() << std::endl;
	out << "      - server client body size limit : " << c.getClientBodySizeLimit() << std::endl;
	std::vector<std::string>::iterator It;
	std::vector<std::string> vect = c.getServerName();
	out << "      - server names : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	vect = c.getErrorPage();
	out << "      - error pages : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	std::vector<Location> locations = c.getLocations();
	std::vector<Location>::iterator Itr;
	out << "      - locations : " << std::endl;
	for(Itr = locations.begin(); Itr != locations.end(); Itr++)
		out << *Itr << std::endl;

	out << "=================================================================" << std::endl;
	return(out);
}