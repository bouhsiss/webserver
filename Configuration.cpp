#include "Configuration.hpp"

void trimSpaces(std::string &line) {
	int i = 0;
	while(line[i] && std::isspace(line[i]))
		i++;
	line.erase(0, i);
	i = line.size();
	while(i > 0 && str::isspace(str[i-1]))
		i--;
	str.erase(i);
}

void Configuration::parser(char *configFilePath) {
	std::string conf(configFilePath);
	std::ifstream file(conf);
	if(!file.is_open()) {
		std::cout << "couldn't open file" << std::endl;
		return;
	}
	std::string line;
	while(std::getline(file, line)) {
		//strip comments if found
		int pos = line.find("#");
		if(pos != string::npos)
			line = line.substr(0, pos);

		trimSpaces(line);

		if(line.empty())
			continue ;
		
		//check for opening/closing braces (counting)
		//check for server/location blocks
	}
}