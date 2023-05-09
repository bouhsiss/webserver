#pragma once
#include "Configuration.hpp"
#include "Server.hpp"
#include "Http.hpp"

class ServerFarm {
	public :
		void configure(std::string configFilePath);
		static ServerFarm *getInstance();
		const std::vector<Server>& getServers() const;
		void initServers();
		void runEventLoop();
	private :
		ServerFarm(const ServerFarm &other);
		void operator=(const ServerFarm &other);
		ServerFarm();

		bool isServerActive(Server &server);
		void areServersDuplicated();

		static ServerFarm *instancePtr;

		Configuration _config;
		std::vector<Server> _servers;
		std::map<int, Server *> _activeServers; // a map with the listening socket as key, and a pointer to the socket's server as a value
		std::vector<int> clientSockets;
		std::vector<int> writeSockets;
		std::map<int, Server *> _clientSockets;
		std::map<int, Server *> _writeSockets;
};

std::ostream& operator<<(std::ostream &out, ServerFarm& c);


// to be noted : might change the way server duplicates are handled depending if we're keeping the server_name or not