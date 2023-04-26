#pragma once
#include "Configuration.hpp"
#include "Server.hpp"

class ServerFarm {
	public :
		void configure(std::string configFilePath);
		static ServerFarm *getInstance();
		const std::vector<Server>& getServers() const;

	private :
		ServerFarm(const ServerFarm &other);
		void operator=(const ServerFarm &other);
		ServerFarm();

		static ServerFarm *instancePtr;

		Configuration _Config;
		std::vector<Server> _Servers;
};

std::ostream& operator<<(std::ostream &out, ServerFarm& c);