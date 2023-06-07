/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerFarm.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassan <hassan@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:42:24 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/27 15:22:44 by hassan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerFarm.hpp"

std::vector<Server*>& ServerFarm::getServers() {return(_servers);}
const std::map<int, Server *>& ServerFarm::getActiveServers() const {return(_activeServers);}
const std::map<int, Server *>& ServerFarm::getClientSockets() const {return(_clientSockets);}
const std::map<std::string, std::string>& ServerFarm::getMIMEtypes() {return(_MIMEtypes);}
std::map<std::string, std::string>&	ServerFarm::getReverseMIMEtypes(){return _ReverseMIMEtypes;}

ServerFarm* ServerFarm::instancePtr = NULL;


/*
	a function to read the mime types file and store them in two maps :
		- _Mimetype <std::string, std::string> ==> key - extension , value - MIME type
		- _ReverseMIMEtypes <std::string, std::string> ==> key - MIME type, value - extension
*/
void ServerFarm::readMIMEtypes() {
	std::ifstream MIME_file(MIME_TYPES_FILE_PATH);
	if(!MIME_file)
		throw(Http::ServerFarmErrorException("failed to open mime.types file"));
	std::string line;
	while(std::getline(MIME_file, line)) {
		if(line.find("types {") != std::string::npos ||
			line.find("}") != std::string::npos ||
			line.empty() || line[0] == '#')
				continue ;
		std::istringstream iss(line);
		std::string mimeType;
		iss >> mimeType;

		std::string extension;
		while(iss >> extension){
			if(_ReverseMIMEtypes[mimeType].empty())
				_ReverseMIMEtypes[mimeType] = extension;
			_MIMEtypes[extension] = mimeType;
		}
	}
	MIME_file.close();
}

ServerFarm::ServerFarm() {
	FD_ZERO(&_readFds);
	FD_ZERO(&_writeFds);
	readMIMEtypes();
}

/*
	creates one instance of the class if it doesn't exist, and return it . else it return the created instance
*/
ServerFarm *ServerFarm::getInstance() {
	if (instancePtr == NULL) {
		instancePtr = new ServerFarm();
		return(instancePtr);
	}
	else {
		return(instancePtr);
	}
}

ServerFarm::~ServerFarm() {
	for(size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "deleted" << std::endl;
		delete _servers[i];
	}
	delete instancePtr;
}

/*
	parse the configuration file and store  the configuration in a vector of Server class instances
*/
void ServerFarm::configure(std::string configFilepath) {
	this->_servers = _config.parse(configFilepath);
	areServersDuplicated();
}

/*
	initialize all servers by creating a socket, binding and listening 
*/
void ServerFarm::initServers() {
	for(size_t i = 0; i < _servers.size(); i++)
	{
		if(!isServerActive(*_servers[i]))
		{
			_servers[i]->setupListenSocket(); // check whether there's an already open listening socket listening to this host:port
			_activeServers.insert(std::make_pair(_servers[i]->getListenSocket(), _servers[i]));
		}
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


/*
	check fo server duplicates (same host, port and server_name)
*/
void ServerFarm::areServersDuplicated() {
	for(size_t i = 0; i < _servers.size(); i++) {
		for(size_t j = i + 1 ; j < _servers.size(); j++) {
			if(_servers[i]->getHost() == _servers[j]->getHost() &&
			_servers[i]->getPort() == _servers[j]->getPort() &&
			_servers[i]->getServerName() == _servers[j]->getServerName())
				throw(Http::ConfigFileErrorException("server : " + _servers[i]->getHost() + ":" + _servers[i]->getPort() + " is duplicated."));
		}
	}
}

void ServerFarm::handleResponse(fd_set *tmpWriteFds) {
	std::map<int, Response *>::iterator It;
	std::vector<int> keysToErase;
	for(It = _writeSockets.begin(); It != _writeSockets.end(); ++It) {
		int writeSock = It->first;
		if(It->second->getRequest().request_is_ready() == true) {
			if(FD_ISSET(writeSock, tmpWriteFds)) {
				try {
					It->second->sendResponse();
				}
				catch(const std::exception& e) {
					// exception are thrown mainly when a file/directory fails to open
					Response* response = new Response(It->second->getRequest(), writeSock);
					_writeSockets[writeSock] = response;
				}
				if(It->second->sendFailed()) {
					// when send fails the client is dropped and the connection is closed
					keysToErase.push_back(writeSock);
					FD_CLR(writeSock, &_writeFds);
					close(writeSock);
					std::cout << RED << "send() failed" << RESET << std::endl;
				}
				if(It->second->isResponseSent() == true) {
					// once the response is sent the socket fd is untrackable and removed from the write fd_set and the connection is closed
					std::cout << GREEN << "response sent to socket : " << writeSock << RESET << std::endl;
					keysToErase.push_back(writeSock);
					FD_CLR(writeSock, &_writeFds);
					close(writeSock);
				}
			}
		}
	}
	for(size_t i = 0; i < keysToErase.size(); i++)
	{
		delete(_writeSockets[keysToErase[i]]);
		_writeSockets.erase(keysToErase[i]);
		_clientSockets.erase(keysToErase[i]);
	}
}

void ServerFarm::handleNewClient(fd_set *tmpReadFds, int *fdmax) {
	std::map<int, Server *>::iterator It;
	for(It = _activeServers.begin(); It != _activeServers.end(); ++It) {
		int sockFd = It->first;
		if(FD_ISSET(sockFd, tmpReadFds)) {
			int clientSocket = accept(sockFd, NULL, NULL);
			if(clientSocket < 0)
				throw(Http::NetworkingErrorException(strerror(errno)));
			FD_SET(clientSocket, &_readFds);
			fcntl(clientSocket, F_SETFL, O_NONBLOCK);
			_clientSockets.insert(std::make_pair(clientSocket, It->second));
			if(clientSocket > *fdmax)
				*fdmax = clientSocket;
		}
	}
}


void ServerFarm::handleRequest(fd_set *tmpReadFds) {
	std::map<int, Server*>::iterator It;
	std::vector<int> keysToErase;
	for(It = _clientSockets.begin(); It != _clientSockets.end(); It++) {
		int clientSock = It->first;
		if(FD_ISSET(clientSock, tmpReadFds)) {
			char read[1024];
			int bytesReceived = recv(clientSock, read, 1024, 0);
			if(bytesReceived < 0)
			{
				// when recv fails the client is dropped and the connection is closed
				keysToErase.push_back(clientSock);
				FD_CLR(clientSock, &_readFds);
				std::cout << RED << "recv() failed" << RESET << std::endl;
				close(clientSock);
			}
			else if(!bytesReceived) {
				// when the client closes the connection. the client is dropped and the connection is closed
				keysToErase.push_back(clientSock);
				FD_CLR(clientSock, &_readFds);
				std::cout << RED << "client closed connection" << RESET << std::endl;
				close(clientSock);
			}
			else {
				std::string reqData(read, bytesReceived);
				/*
					if the request is a new request a new intance is created and a call to process request is made , else the function proceed directly to the request processing . 
					once a request is ready the socket is cleared from the read fd_set and inserted into the write fd_set and
				*/
				if(_writeSockets.find(clientSock) != _writeSockets.end()) {
					_writeSockets[clientSock]->getRequest().proccess_Request(reqData);
					if(_writeSockets[clientSock]->getRequest().request_is_ready()) {
						FD_CLR(clientSock, &_readFds);
						FD_SET(clientSock, &_writeFds); 
					}
				}
				else {
					Request *request = new  Request(It->second->getHost(), It->second->getPort());
					request->proccess_Request(reqData);
					Response *response = new Response(*request, clientSock);
					if(request->request_is_ready())
					{
						FD_CLR(clientSock, &_readFds);
						FD_SET(clientSock, &_writeFds);
					}
					_writeSockets.insert(std::make_pair(clientSock, response));
				}
			}
		}
	}
	for(size_t i = 0 ; i < keysToErase.size(); i++)
		_clientSockets.erase(keysToErase[i]);
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
		{
			std::cout << "select failed" << std::endl;
			throw(Http::NetworkingErrorException(strerror(errno)));
		}
		
		// once the request is ready the function proceed to forge a response and send it to the client
		handleResponse(&tmpWriteFds);
		// setup and accept the new client connection
		handleNewClient(&tmpReadFds, &fdmax);
		// receive the data and process the request
		handleRequest(&tmpReadFds);
	}
}


std::ostream& operator<<(std::ostream &out, ServerFarm& c) {
	std::vector<Server*>::iterator It;
	std::vector<Server*> servers = c.getServers();
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