#include"Http.hpp"


int main(int ac, char **av) {
	if(ac != 2)
		std::cout << "Usage : ./webserv <config file>" << std::endl;
	else {
		Configuration globalConfig;
		globalConfig.parser(av[1]);
		std::cout << globalConfig << std::endl;
	}
}