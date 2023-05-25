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
#include <fcntl.h>
#include <algorithm>
#include "ServerFarm.hpp"
#include <limits.h>
#include <stdlib.h>

class ServerFarm;

class Request : public HttpMessage {
	public :
		Request();
		Request(std::string request_host, std::string request_port);
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();
        std::string getMethod()const;
        std::string getRequestURI()const;
		//parsing is done here
		void proccess_Request(std::string req_data);
		bool request_is_ready();
		//getters
		std::string getHttp_version()const;
		int			getServerIndex()const;
		int			getStatusCode() const;
		std::string getLocationIndex()const;
		std::string	getresourceType()const;
		std::string	getRequestedresource()const;
		std::string getUploadFilename()const;
		std::string getUploadFile()const;
		std::string getFilenameExtension()const;
		void print();
        bool is_location_has_redirection();


	private :

		bool check_for_forbidden_chars(std::string)const;
		void get_matched_location_for_request_uri();
        bool is_method_allowed_in_location();
        void check_which_requested_method();
		//----------- supported methods
        void GET();
        void POST();
        void DELETE();

        //----------- helper functions
		bool search_for_indexfile(const char *dir_path);
		void upload_resource();
        bool get_requested_resource();
        bool delete_all_folder_content();
        std::string get_resource_type();
        bool is_uri_has_slash_in_end();
        bool is_dir_has_index_file();
        std::string get_auto_index();
        bool if_location_has_cgi();
        bool if_location_support_upload();
        bool has_write_acces_on_folder();
		void unchunk_body();
		void handle_multipart_form_data();
		bool check_forbidden_path();
		bool indexFileExists(const char* dir_path, std::string &filename);

		//function to run cgi
		void run_cgi();
		void set_cgi_env();
		void prepare_env();
    	void clean_cgi_output();



		std::string 	_method;
		std::string 	_RequestURI;
		std::string 	_http_v;
		int 			_status_code;
		ServerFarm 		*_sf;
		int 			_server_index;
		std::string		_location_index;
		std::string 	_req_host;
		std::string 	_req_port;
		std::string 	_resource_type;
		std::string 	_requested_resource;
		//for POST method
		std::string		_upload_filename;
		std::fstream	_upload_file;
		std::string		_filename_extension;
		//for cgi
		//maybe i will change this later to a struct or something
		std::string		_script_name;
		std::string		_query_string;
		std::string		_path_info;
		std::string		_path_translated;
		std::string		_remote_host;
		std::string		_remote_addr;
		std::string		_auth_type;
		std::string		_remote_user;
		std::string		_remote_ident;
		//for cgi out file /check them for get/post
		std::fstream	_cgi_output;
		std::string		_cgi_output_filename;
};
