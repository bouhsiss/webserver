#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include "Server.hpp"
#include "Location.hpp"

class Configuration {
	public :
		void parser(char *configFilePath);
	private :
		std::vector<Server> _Servers;	
};