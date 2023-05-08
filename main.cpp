#include"Http.hpp"
#include "ServerFarm.hpp"


int main(int ac, char **av) {
	if(ac > 2)
		std::cout << "Usage : ./webserv <config file>" << std::endl;
	else {
		const char *configFilePath = (ac == 2) ? av[1] : DEFAULT_CONFIG_FILE;
		ServerFarm *webserv = ServerFarm::getInstance();
		try {
			webserv->configure(configFilePath);
			webserv->initServers();
		}
		catch(Http::ConfigFileErrorException& e) {
			std::cout << YELLOW << "Config : " << e.what() << RESET << std::endl;
		}
		catch(Http::NetworkingErrorException& e) {
			std::cout << YELLOW << "Networking : " << e.what() << RESET << std::endl;
		}
	}
}
