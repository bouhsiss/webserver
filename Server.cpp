#include"Server.hpp"

//***===  setters  ===***
void Server::addLocation(Location Location) {_Locations.push_back(Location);}
void Server::setListen(std::string listen) {_listen = listen;}
void Server::setServerName(std::string serverName) {_server_name = serverName;}
void Server::setHost(std::string host) {_host = host;}
void Server::setErrorPage(std::string errorPage) {_error_page = errorPage;}
void Server::setClienBodySizeLimit(std::string clientBodySizeLimit) {_client_body_size_limit = clientBodySizeLimit;}

//***===  getters  ===***
const std::vector<Location>& Server::getLocations() {return(_Locations);}
const std::string Server::getListen() {return(_listen);}
const std::string Server::getServerNAme() {return(_server_name);}
const std::string Server::getHost() {return(_host);}
const std::string Server::getErrorPage() {return(_error_page);}
const std::string Server::getClientBodySizeLimit() {return(_client_body_size_limit);}