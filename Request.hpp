#pragma once
#include "HttpMessage.hpp"
#include <iostream>
#include <string>

class Request : public HttpMessage {
	public :
		Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();

		void parseRequest(std::string RequestData);
	private :
		std::string method;
		std::string RequestURI;
};