#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
// #include "Server.hpp"

class HttpMessage {
	public :
		HttpMessage();
		//parsing is done here
		HttpMessage(std::string& Message);
		HttpMessage(HttpMessage const& other);
		HttpMessage& operator=(HttpMessage const& other);
		~HttpMessage();

		const std::string& getStartLine() const;
		const std::string& getBody() const;
		const std::map<std::string, std::string>& getHeaders() const;

		void setStartLine(std::string StartLine);
		void setBody(std::string Body);
		void setHeaders(std::string name, std::string value);

	protected :
		std::string _StartLine;
		std::map<std::string, std::string> _Headers;
		std::string _Body;
};