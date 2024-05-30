#include"Http.hpp"
#include "ServerFarm.hpp"

void signalHandler(int signum) {
	std::cout << RED << "Interrupt signal (" << signum << ") received." << std::endl;
	ServerFarm *webserv = ServerFarm::getInstance();

	std::map<int, Server *> activeServers = webserv->getActiveServers();
	std::map<int, Server *> clientSockets = webserv->getClientSockets();
	std::map<int, Server*>::iterator It;
	for(It = activeServers.begin(); It != activeServers.end(); It++)
		close(It->first);
	for(It = clientSockets.begin(); It != clientSockets.end(); It++)
		close(It->first);
}

int main(int ac, char **av) {
	if(ac > 2)
		std::cout << "Usage : ./webserv <config file>" << std::endl;
	else {
		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, signalHandler);
		const char *configFilePath = (ac == 2) ? av[1] : DEFAULT_CONFIG_FILE;
		try {
		    ServerFarm *webserv = ServerFarm::getInstance();
			webserv->configure(configFilePath);
			webserv->initServers();
			webserv->runEventLoop();
		}
		catch(Http::ConfigFileErrorException& e) {
			std::cout << YELLOW << "Config : " << e.what() << RESET << std::endl;
		}
		catch(Http::NetworkingErrorException& e) {
			std::cout << YELLOW << "Networking : " << e.what() << RESET << std::endl;
		}
		catch(Http::ServerFarmErrorException& e){
			std::cout << YELLOW << "ServerFarm : " << e.what() << RESET << std::endl;
		}
		catch(Http::ResponseErrorException& e){
			std::cout << YELLOW << "ServerFarm : " << e.what() << RESET << std::endl;
		}
	}
}
