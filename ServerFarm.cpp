#include "ServerFarm.hpp"

const std::vector<Server>& ServerFarm::getServers() const {return(_Servers);}


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
	_Config.parse(configFilepath, this->_Servers);
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