#pragma once
#include "HttpMessage.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>
#include "ServerFarm.hpp"
class ServerFarm;

class Request : public HttpMessage {
	public :
		Request();
		//parsing is done here
		Request(std::string request_host, std::string request_port);
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();
        std::string getMethod()const;
        std::string getRequestURI()const;
		void proccess_Request(std::string req_data);
		bool request_is_ready();


	private :

		bool check_for_forbidden_chars(std::string)const;
		void get_matched_location_for_request_uri();
        bool is_location_has_redirection();
        bool is_method_allowed_in_location();
        void check_which_requested_method();
		//----------- supported methods
        void GET();
        void POST();
        void DELETE();

        //----------- helper functions
        bool get_requested_resource();
        bool delete_all_folder_content();
        std::string get_resource_type();
        bool is_uri_has_slash_in_end();
        bool is_dir_has_index_file();
        std::string get_auto_index();
        bool if_location_has_cgi();
        bool if_location_support_upload();
        bool has_write_acces_on_folder();
		//function to run cgi
		void run_cgi();
		//chunked request
		void add_chunk(std::string chunk);
		//check if request arrived fully


		std::string _method;
		std::string _RequestURI;
		std::string _http_v;
		int 		_status_code;
		ServerFarm 	*_sf;
		int 		_server_index;
		std::string	_location_index;
		std::string _req_host;
		std::string _req_port;
		std::string _resource_type;
		std::string _requested_resource;
		//for chunked request
		//also for chunked request request is complete when you reach the length in the content_length request header
		// size_t current_content_length;
};