#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdio>
#include "Http.hpp"

// #include "Server.hpp"

class HttpMessage {
	public :
		HttpMessage();
		//parsing is done here
		HttpMessage(HttpMessage const& other);
		HttpMessage& operator=(HttpMessage const& other);
		~HttpMessage();
		bool getb() const { return(_b_complete);}
	protected :
		void parse();
		void setHeaders(std::string name, std::string value);
		std::string random_filename(); 

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