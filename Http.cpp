#include "Http.hpp"

void Http::tokenize(std::string const &str, const char* delim, std::vector<std::string>& out){
	char *token = strtok(const_cast<char*>(str.c_str()), delim);
	while(token != NULL) {
		out.push_back(std::string(token));
		token = strtok(NULL, delim);
	}
}


void Http::trimSpaces(std::string &line) {
	line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
	line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
}

bool Http::strIsNumber(const std::string &s) {
	try {
		std::stoi(s);
		return (true);
	}
	catch (...) {
		return(false);
	}
}

void Http::printAddr(struct addrinfo *peerAddress) {
	struct addrinfo *address = peerAddress;
	char addressBuffer[100];
	getnameinfo(address->ai_addr, address->ai_addrlen, addressBuffer,sizeof(addressBuffer), 0, 0, NI_NUMERICHOST);
	std::cout << YELLOW << "Remote addresss is : " << addressBuffer << RESET << std::endl;
}