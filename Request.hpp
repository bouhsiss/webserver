#pragma once
#include <iostream>

class Request {
	public :
		Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();

		void parseRequest(std::string RequestData);
};