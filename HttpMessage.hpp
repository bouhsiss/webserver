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
		HttpMessage(HttpMessage const& other);
		HttpMessage& operator=(HttpMessage const& other);
		~HttpMessage();
	protected :
		void parse();
		void append_chunk(std::string chunk);
		void setHeaders(std::string name, std::string value);

		std::string 						_StartLine;
		std::map<std::string, std::string> 	_Headers;
		std::fstream 						_Body;
		size_t								_body_length;
		std::string							_filename;
		std::string							_message;
		bool								_sl_complete;
		bool								_hd_complete;
		bool								_b_complete;
};