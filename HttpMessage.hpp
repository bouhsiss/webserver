#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdio>

// #include "Server.hpp"

class HttpMessage {
	public :
		HttpMessage();
		//parsing is done here
		HttpMessage(std::string& Message);
		HttpMessage(HttpMessage const& other);
		HttpMessage& operator=(HttpMessage const& other);
		~HttpMessage();
		void append_chunk(std::string chunk);
		void setHeaders(std::string name, std::string value);

	protected :
		std::string 						_StartLine;
		std::map<std::string, std::string> 	_Headers;
		std::fstream 						_Body;
		int									_body_length;
		char 								*_filename;
};