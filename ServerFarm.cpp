#include "ServerFarm.hpp"

const std::vector<Server>& ServerFarm::getServers() const {return(_servers);}


ServerFarm* ServerFarm::instancePtr = NULL;

ServerFarm::ServerFarm() {}

ServerFarm *ServerFarm::getInstance() {
	if (instancePtr == NULL) {
		instancePtr = new ServerFarm();
		return(instancePtr);
	}
	else {
		return(instancePtr);
	}
}

void ServerFarm::configure(std::string configFilepath) {
	this->_servers = _config.parse(configFilepath);
	areServersDuplicated();
}

void ServerFarm::initServers() {
	std::vector<Server>::iterator It;
	for(It = _servers.begin(); It != _servers.end(); It++)
	{
		if(isServerActive(*It))
			It->setupListenSocket(); // check whether there's an already open listening socket listening to this host:port
		_activeServers.insert(std::make_pair(It->getListenSocket(), &(*It)));
	}
}

bool ServerFarm::isServerActive(Server &server) {
	std::map<int, Server *>::iterator It;
	for(It = _activeServers.begin(); It != _activeServers.end(); It++) {
		if(server.getHost() == It->second->getHost() && server.getPort() == It->second->getPort())
			return(true);
	}
	return(false);
}

void ServerFarm::areServersDuplicated() {
	for(size_t i = 0; i < _servers.size(); i++) {
		for(size_t j = i + 1 ; j < _servers.size(); j++) {
			if(_servers[i].getHost() == _servers[j].getHost() &&
			_servers[i].getPort() == _servers[j].getPort() &&
			_servers[i].getServerName() == _servers[j].getServerName())
				throw(Http::ConfigFileErrorException("server : " + _servers[i].getHost() + ":" + _servers[i].getPort() + " is duplicated."));
		}
	}
}

std::ostream& operator<<(std::ostream &out, ServerFarm& c) {
	std::vector<Server>::iterator It;
	std::vector<Server> servers = c.getServers();
	out << "======================== CONFIGURATION ========================" << std::endl;
	for(It = servers.begin(); It != servers.end(); It++) {
		int serverCount = 0;
		out << "------------- SERVER : " << serverCount << " -------------" << std::endl;
		out << *It;
		serverCount++;
	}
	out << "===============================================================" << std::endl;
	return(out);
}