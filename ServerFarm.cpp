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


// the main select() event loop
void ServerFarm::runEventLoop() {
	fd_set readFds;
	fd_set writeFds;
	int fdmax = 0;

	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	// add servers listening sockets to the readFds
	std::map<int ,Server *>::iterator It;
	for(It = _activeServers.begin(); It != _activeServers.end(); It++) {
		FD_SET(It->first, &readFds);
		fdmax = std::max(It->first, fdmax);
	}
	// reason we have tmp sets is because select() changes the set you pass into it
	fd_set tmpReadFds;
	fd_set tmpWriteFds;
	while(true) {
		tmpReadFds = readFds;
		if(select(fdmax + 1, &tmpReadFds, &tmpWriteFds, NULL, NULL) == -1)
			throw(Http::NetworkingErrorException(strerror(errno)));
		for(It = _activeServers.begin(); It != _activeServers.end(); It++) {
			int sockFd = It->first;
			if(FD_ISSET(sockFd, &tmpReadFds)) {
				// code to handle new connection and add the fd to the read fd set and  to the client Sockets vector
			}
		}
		std::vector<int>::iterator vectIt;
		for(vectIt = clientSockets.begin(); vectIt != clientSockets.end(); vectIt++) {
			if(FD_ISSET(*vectIt, &readFds)) {
				// code to handle the request and add the fd to the write fd set and to the write sockets
			}
		}

		for(vectIt = writeSockets.begin(); vectIt != writeSockets.end(); vectIt++) {
			if(FD_ISSET(*vectIt, &tmpWriteFds)) {
				// code to send response to client and remove the fd from the vector and the write fds
			}
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