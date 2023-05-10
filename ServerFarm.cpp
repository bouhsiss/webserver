/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerFarm.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:42:24 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:42:26 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerFarm.hpp"

const std::vector<Server>& ServerFarm::getServers() const {return(_servers);}


ServerFarm* ServerFarm::instancePtr = NULL;

ServerFarm::ServerFarm() {
	FD_ZERO(&_readFds);
	FD_ZERO(&_writeFds);
}

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
		if(!isServerActive(*It))
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

std::string defaultResponse() {
	std::ifstream html_file("default.html");
	std::stringstream buffer;
	buffer << html_file.rdbuf();
	html_file.close();
	std::string html_contents = buffer.str();

	std::string response_headers = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: " + std::to_string(html_contents.length()) + "\r\n"
                               "\r\n";

	std::string response = response_headers + html_contents;
	return (response);
}


void ServerFarm::handleResponse(fd_set *tmpWriteFds) {
	std::map<int, Server*>::iterator It;
	for(It = _writeSockets.begin(); It != _writeSockets.end(); It++) {
		int writeSock = It->first;
		if(FD_ISSET(writeSock, tmpWriteFds)) {
			if(FD_ISSET(writeSock, tmpWriteFds)) {
				// code to handle response to client and remove the fd from the or and the write fds
				if(send( writeSock, defaultResponse().c_str(), defaultResponse().length(), 0 ) < 0)
				{	FD_CLR(writeSock, &_writeFds);
					close(writeSock);
					throw(Http::NetworkingErrorException("send failed"));
				}
				FD_CLR(writeSock, &_writeFds);
			}
		}
	}
}

void ServerFarm::handleNewClient(fd_set *tmpReadFds, int *fdmax) {
	std::map<int, Server *>::iterator It;
	for(It = _activeServers.begin(); It != _activeServers.end(); It++) {
		int sockFd = It->first;
		if(FD_ISSET(sockFd, tmpReadFds)) {
			// code to handle new connection and add the fd to the read fd set and  to the client Sockets vector
			int clientSocket = accept(sockFd, NULL, NULL);
			if(clientSocket < 0)
				throw(Http::NetworkingErrorException(strerror(errno)));
			FD_SET(clientSocket, &_readFds);
			_clientSockets.insert(std::make_pair(clientSocket, It->second));
			// clientSockets.push_back(clientSocket);
			if(clientSocket > *fdmax)
				*fdmax = clientSocket;
			std::cout << CYAN << "new connection from server : " << It->second->getHost() << ":" << It->second->getPort() << RESET << std::endl;
		}
	}
}

void ServerFarm::handleRequest(fd_set *tmpReadFds) {
	std::map<int, Server*>::iterator It;
	for(It = _clientSockets.begin(); It != _clientSockets.end(); It++) {
		int clientSock = It->first;
		if(FD_ISSET(clientSock, tmpReadFds)) {
			// code to handle the request and add the fd to the write fd set and to the write sockets
			int clientBodySizeLimit = It->second->getClientBodySizeLimit();
			char read[clientBodySizeLimit];
			int bytesReceived = recv(clientSock, read, clientBodySizeLimit, 0);
			if(bytesReceived < 0)
				throw(Http::NetworkingErrorException("recv() failed"));
			else if(!bytesReceived) {
				FD_CLR(clientSock, &_readFds);
				close(clientSock);
			}
			else {
				std::cout << "==================== REQUEST ========================= " << std::endl << std::endl;
				std::string str(read, bytesReceived);
				std::cout << MAGENTA << str << RESET << std::endl;
				std::cout << "=======================================================" << std::endl;
				FD_SET(clientSock, &_writeFds); 
				_writeSockets.insert(std::make_pair(clientSock, It->second));
			}

		}
	}
}

// the main select() event loop
void ServerFarm::runEventLoop() {
	int fdmax = 0;

	// add servers listening sockets to the readFds
	std::map<int ,Server *>::iterator It;
	for(It = _activeServers.begin(); It != _activeServers.end(); It++) {
		FD_SET(It->first, &_readFds);
		fdmax = std::max(It->first, fdmax);
	}
	//tmp sets because select() changes the set you pass into 
	fd_set tmpReadFds;
	fd_set tmpWriteFds;
	while(true) {
		tmpReadFds = _readFds;
		tmpWriteFds = _writeFds;
		if(select(fdmax + 1, &tmpReadFds, &tmpWriteFds, NULL, NULL) == -1)
			throw(Http::NetworkingErrorException(strerror(errno)));
		
		// priority to this loop, since the writeFds from previous iteration won't  notify until the next one
		handleResponse(&tmpWriteFds);
		handleNewClient(&tmpReadFds, &fdmax);
		handleRequest(&tmpReadFds);
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
