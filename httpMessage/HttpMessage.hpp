#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
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
		void 	read_first_chunk();
		void	read_new_chunk();
		void	append_chunk();

		std::string 						_startLine;
		std::map<std::string, std::string> 	_Headers;
		std::fstream 						_Body;
		size_t								_body_length;
		std::string							_filename;
		std::string							_message;
		bool								_sl_complete;
		bool								_hd_complete;
		bool								_b_complete;
		int									_chunk_size;
		int									_flag;
		int 								_bytes_read;
};