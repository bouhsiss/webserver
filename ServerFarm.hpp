#pragma once
#include "Configuration.hpp"
#include "Server.hpp"

class ServerFarm {
	public :
		void configure(std::string configFilePath);
		static ServerFarm *getInstance();
		const std::vector<Server>& getServers() const;
		void initServers();
	private :
		ServerFarm(const ServerFarm &other);
		void operator=(const ServerFarm &other);
		ServerFarm();

		bool isServerActive(Server &server);
		void areServersDuplicated();

		static ServerFarm *instancePtr;

		Configuration _config;
		std::vector<Server> _servers;
		std::map<int, Server *> _activeServers;

};

std::ostream& operator<<(std::ostream &out, ServerFarm& c);