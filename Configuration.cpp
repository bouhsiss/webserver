#include "Configuration.hpp"

void Configuration::parser(char *configFilePath) {
	std::string conf(configFilePath);
	std::ifstream file(conf);
	if(!file.is_open()) {
		std::cout << "couldn't open file" << std::endl;
		return;
	}
	std::string line;
	while(std::getline(file, line)) {
		
	}
}