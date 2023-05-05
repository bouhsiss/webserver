#include"Server.hpp"

const std::map<std::string, Location *>& Server::getLocations() {return(_Locations);}
const std::string& Server::getServerName() {return(_server_name);}
const std::string& Server::getHost() const {return(_host);}
const std::string& Server::getPort() const {return(_port);}
const std::vector<std::string>& Server::getErrorPage() {return(_error_page);}
const int& Server::getClientBodySizeLimit() const {return(_client_body_size_limit);}

void Server::setPort(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !Http::strIsNumber(tokens[0]) || ( std::stoi(tokens[0]) < 1023 && std::stoi(tokens[0]) != 80 && std::stoi(tokens[0]) != 443) || !this->_port.empty())
		throw(Http::ConfigFileErrorException("Invalid port directive"));
	this->_port = tokens[0];
}

void Server::setServerName(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1  || !_server_name.empty())
		throw(Http::ConfigFileErrorException("Invalid server name directive"));
	this->_server_name = tokens[0];
}

void Server::setHost(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !this->_host.empty())
		throw(Http::ConfigFileErrorException("invalid host directive"));
	this->_host = tokens[0];
}

void Server::setErrorPage(std::vector<std::string> const &tokens) {
	if(tokens.empty())
		throw(Http::ConfigFileErrorException("invalid error_page directive"));
	if(std::find(_error_page.begin(), _error_page.end(), tokens[0]) != _error_page.end())
		throw(Http::ConfigFileErrorException("duplicated value : " + tokens[0]));
	this->_error_page = tokens;
}

void Server::setClientBodySizeLimit(std::vector<std::string> const &tokens) {
	if(tokens.size() != 1 || !Http::strIsNumber(tokens[0]) || std::stoi(tokens[0]) < 0 || this->_client_body_size_limit != -1)
		throw(Http::ConfigFileErrorException("Invalid client body size limit directive"));
	this->_client_body_size_limit = std::stoi(tokens[0]);
}


Server::Server() {
	this->_port = "";
	this->_host = "";
	this->_client_body_size_limit = -1;
	this->_server_name = "";
}



void Server::isServerValid() {
	if(_host.empty())
		_host = "127.0.0.1";
	if(_port.empty())
		_port = 80;
	if(_client_body_size_limit == -1)
		throw(Http::ConfigFileErrorException("incomplete Server Configuration : client_body_size_limit directive missing"));
	std::map<std::string, Location *>::iterator It;
	for(It = _Locations.begin(); It != _Locations.end(); It++)
		It->second->isLocationValid();
}

void Server::startListening() {
	struct addrinfo hints, *res;
	std::cout << GREEN << "Configuring local address..." << RESET << std::endl;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int status = getaddrinfo(_host.c_str(), _port.c_str(), &hints, &res);
	if(status)
		throw(Http::ConfigFileErrorException("getaddrinfo error :" + std::string(gai_strerror(status))));
	std::cout << GREEN << "Creating listening socket.." << RESET << std::endl;
	listenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(listenSocket < 0)
		throw(Http::ConfigFileErrorException("socket() failed"));
	std::cout << GREEN << "Binding socket to local address.. " << RESET << std::endl;
	if(bind(listenSocket, res->ai_addr, res->ai_addrlen))
		throw(Http::ConfigFileErrorException("bind() failed.."));
	freeaddrinfo(res);
	std::cout << GREEN << "Listening on " << RESET << std::endl;
	Http::printAddr(res);
	if(listen(listenSocket, 10) < 0)
		throw(Http::ConfigFileErrorException("listen() failed") );
}

std::ostream& operator<<(std::ostream &out, Server &c) {
	out << "======================== SERVER BLOCK ========================" << std::endl;
	out << "      - server host : " << c.getHost() << std::endl;
	out << "      - server port : " << c.getPort() << std::endl;
	out << "      - server client body size limit : " << c.getClientBodySizeLimit() << std::endl;
	out << "      - server server name : " << c.getServerName() << std::endl;
	std::vector<std::string>::iterator It;
	std::vector<std::string> vect = c.getErrorPage();;
	out << "      - error pages : " << std::endl;
	for(It = vect.begin(); It != vect.end(); It++)
		out << "                        * " << *It << std::endl;
	std::map<std::string, Location *> locations = c.getLocations();
	std::map<std::string, Location *>::iterator Itr;
	out << "      - locations : " << std::endl;
	for(Itr = locations.begin(); Itr != locations.end(); Itr++)
		out << *Itr->second << std::endl;

	out << "=================================================================" << std::endl;
	return(out);
}