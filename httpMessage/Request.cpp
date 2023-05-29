#include "Request.hpp"

Request::Request() {}
Request::Request(std::string request_host, std::string request_port):_sf(ServerFarm::getInstance()),_req_host(request_host),_req_port(request_port){
	_method = "";
	_RequestURI = "";
	_http_v = "";
	_status_code = -1;
	_server_index = -1;
	_location_index = "";
	_resource_type = "";
	_requested_resource = "";
	_upload_filename = "";
	_filename_extension = "";
	_upload_done=false;
	_b_complete = false;
	_cgi_flag=false;
}

Request::Request(const Request& other) {
	(void)other;
}

Request& Request::operator=(const Request& other) {
	(void)other;
	return(*this);
}

Request::~Request() {
    remove(_cgi_output_filename.c_str());
	remove(_filename.c_str());
	remove(_tmp_filename.c_str());
	remove(_cgi_tmpfilename.c_str());
}


void Request::normalizePath() {
	std::string normalizedPath = _RequestURI;
	for(size_t i = _RequestURI.length() - 1; i > 0; --i) {
		if(normalizedPath[i] == '/' && normalizedPath[i - 1] == '/')
			normalizedPath.erase(i, 1);
	}
	std::stringstream decodedPath;
    std::string queryString;
    std::string::const_iterator it = normalizedPath.begin();

    while (it != normalizedPath.end()) {
        if (*it == '%') {
            if (std::distance<std::string::const_iterator>(it, normalizedPath.end()) >= 3) {
                std::string hex = normalizedPath.substr(std::distance<std::string::const_iterator>(normalizedPath.begin(), it) + 1, 2);

                char decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
                decodedPath << decodedChar;
                std::advance(it, 3);
            } else {
                break;
            }
        } else if (*it == '?') {
            decodedPath << *it;
            ++it;

            while (it != normalizedPath.end()) {
                queryString += *it;
                ++it;
            }
            break;
        } else {
            decodedPath << *it;
            ++it;
        }
    }

	_RequestURI= decodedPath.str() + queryString;
}

void Request::proccess_Request(std::string req_data){
	
    _bytes_read = req_data.length();
    _message+=req_data;
    if (_b_complete==false)
        parse();
    //initial status code (if status code remain -1 that means no errors found at this stage)
    //if its not a valid httpmessage stop here
    if (_b_complete==true)
    {
        //check if host header field
        if (_Headers.find("Host")==_Headers.end())//there is not a host header
        {
            //400 bad request
            _status_code = 400;
        }
        else //choosing the right server to handle the request
        {
				//test the port and ip add of the request against the listen directives of the server blocks
				
				std::map<int, Server*> valid_listen_directive;
				std::map<int, Server*>::iterator It;
				for(size_t i = 0; i < _sf->getServers().size(); i++) {
					if ((_sf->getServers())[i]->getHost() == _req_host && (_sf->getServers())[i]->getPort() == _req_port) {
						valid_listen_directive.insert(std::make_pair(i, _sf->getServers()[i]));
						_server_index = i;
					}
				}
				if(valid_listen_directive.size() != 1) {
					for(It = valid_listen_directive.begin(); It != valid_listen_directive.end(); It++) {
						//test the host request header against the server_name entries of the server blocks that matched ip/port
							if(It->second->getServerName() == _Headers["Host"])
							{
								_server_index = It->first;		
								break;
							}
					}
				}
				if(_sf->getServers()[_server_index]->getServerName() != _Headers["Host"]) {
					//if you didnt find any match pass the request to the default server for ip/port (the first one)
					_server_index = valid_listen_directive.begin()->first;
				}
				else
					std::cerr<<"error: no server found to handle the request"<<std::endl;
			std::istringstream iss(_startLine);
			iss>>_method;
			iss>>_RequestURI;
			iss>>_http_v;
			if (_RequestURI.find("?"))
			{
				_query_string = _RequestURI.substr(_RequestURI.find("?")+1);
				_RequestURI = _RequestURI.substr(0,_RequestURI.find("?"));
			}
			if (_method != "GET" && _method != "DELETE" && _method != "POST")
			{
				//RETURN 501 (Not Implemented)
				_status_code = 501;
			}
			//check http version should be 1.1 does this check matter?
			if (_http_v != "HTTP/1.1")
			{
				//505 not supported version
				_status_code = 505;
			}
			//get requestURI (should be absolute path) if the request URI doesnt exist -->error
			else if (_Headers.find("Transfer-Encoding")!=_Headers.end() && _Headers["Transfer-Encoding"].find("chunked",0)!=std::string::npos)
			{
				//return 501 Not Implemented
				_status_code = 501;
			}
			else if (_Headers.find("Transfer-Encoding")==_Headers.end()&& _Headers.find("Content-Length")==_Headers.end() && _method == "POST")
			{
				//return 400 Bad Request
				_status_code = 400;
			}
			else if (Request::check_for_forbidden_chars(_RequestURI))//request uri contains a char not allowed
			{
				//400 Bad Request
				_status_code = 400;
			}
			else if (_RequestURI.length() > MAX_URI_SIZE)//chrome max uri size
			{
				//414 Request-URI Too Long
				_status_code = 414;
			}
			else if (_sf->getServers()[_server_index]->getClientBodySizeLimit() < _body_length)//request body larger than client max body size in config file
			{
				//413 Request Entity Too Large
				_status_code  = 413;
			}
			else {
				// there's a problem here
				Request::get_matched_location_for_request_uri();
				if (_location_index != "")//location found
				{
					if (Request::is_location_has_redirection())
					{
						//location have redirection like :return 301 /home/index.html
						//301 Moved Permanently
						
						_status_code = 301;
					}
					else
					{
						if (Request::is_method_allowed_in_location()) {
							check_which_requested_method();
							
						}
						else
						{
							//405 Method Not Allowed
							_status_code = 405;
						}
					}
				}
				else//if no location match the request uri
				{
					//404 Not found
					_status_code = 404;
				}
			}
        }
		if (_method == "POST")
			_upload_done=true;
    }
	//debug
	std::cerr<<"finished proccess request"<<std::endl;
	//end debug
	// while(1);
}

std::string Request::getMethod()const{return _method;}
std::string Request::getRequestURI()const{return _RequestURI;}

bool Request::check_for_forbidden_chars(std::string uri){
    std::string forbidden = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    for(size_t i=0;i<uri.length();i++)
    {
        if (forbidden.find(uri[i])==std::string::npos)
            return 1;
    }
	normalizePath(); 
    return 0;
}

void Request::get_matched_location_for_request_uri(){
	std::map<std::string, Location *> tmp;
    //loop the location
    for (std::map<std::string,Location*>::iterator it = _sf->getServers()[_server_index]->getLocations().begin();it != _sf->getServers()[_server_index]->getLocations().end(); it++)
    {
        //if you found a match return add the location to the map

        if (_RequestURI.find(it->first)==0 && (_RequestURI == it->first || _RequestURI[it->first.length()] == '/' || it->first == "/"))
		{
            tmp.insert(std::make_pair(it->first,it->second));
		}
    }
    //get the longest match
    if (tmp.size()!=0)
    {
        _location_index = "";
        for (std::map<std::string,Location*>::iterator it = tmp.begin();it!=tmp.end();it++)
        {
            if (_location_index.size() < it->first.size())
			{
                _location_index = it->first;
			}
        }
    }
}
bool Request::is_location_has_redirection(){
	Location *location = _sf->getServers()[_server_index]->getLocations()[_location_index];
	if(location->getRedirect() != "")
	{
		_requested_resource = location->getRedirect();
		return(true);
	}
    return false;
}
bool Request::is_method_allowed_in_location(){
	Location *location = _sf->getServers()[_server_index]->getLocations()[_location_index];
    for (size_t i=0; i < location->getAllowedMethods().size() ; i++)
    {
        if (_method == location->getAllowedMethods()[i])
            return true;
    }
    return false;
}
void Request::check_which_requested_method(){//check if the body arrived fully before sending response ps: call this function  manualy
    if (_method == "GET")
        GET();
    else if (_method == "POST")
        POST();
    else if (_method == "DELETE")
        DELETE();
}


//----------- supported methods
void Request::GET(){
    if (Request::get_requested_resource())//found
    {
        if (Request::get_resource_type() == "directory")
        {
            if (Request::is_uri_has_slash_in_end())
            {
                if (Request::is_dir_has_index_file())//this directory has an index file
                {
                    if (Request::if_location_has_cgi())//location has cgi
                    {
                        //run cgi on requested file with GET REQUEST_METHOD
                        //return code Depend on cgi 
                        Request::run_cgi();
                    }
                    else //location doensnt have cgi
                    {
                        //return requested file
                        //200 OK
                        _status_code=200;
                    }
                }
                else
                {
                    if (Request::get_auto_index() == "on")
                    {
                        //return auto index of the directory
                        //200 OK
                        _status_code = 200;
                    }
                    else //auto index off
                    {
                        //403 Forbidden
                        _status_code = 403;
                    }
                }
            }
            else//your request uri doesnt have "/" at the end
            {
                //301 Moved Permanenlty
                //make a 301 redirection to request uri with "/" added at the end
                _status_code = 301;
            }
        }
        else //file
        {
            if (Request::if_location_has_cgi())//location has cgi
            {
                //run cgi on requested file with GET REQUEST_METHOD
                //return code depend on cgi
                Request::run_cgi();
            }
            else// location doesnt has cgi
            {
                //return requested file
                //200 OK
                _status_code = 200;
            }
        }

    }
    else //requested resource not found in root
    {
        if (_status_code != 403)
        {
                 //404 not found
			std::cerr<<"get() not found "<<std::endl;
            _status_code = 404;
        }
    }
}


void Request::POST(){
    //dont forget chunked request
    if (Request::if_location_support_upload())//location support upload
    {
        upload_resource();
        //201 created
        _status_code = 201;
    }
    else//location doesnt support upload
    {
        if (Request::get_requested_resource())//found
        {
            if (Request::get_resource_type() == "directory")//directory
            {
                if(Request::is_uri_has_slash_in_end())//yes
                {
                    if (Request::is_dir_has_index_file())//directory has index file
                    {
                        if (Request::if_location_has_cgi())
                        {
                            //run cgi on requested file with POST REQUEST_METHOD
                            //return code depend on cgi
                            Request::run_cgi();
                        }
                        else//location doesnt have cgi
                        {
                            //403 forbidden
                            _status_code = 403;
                        }
                    }
                    else //directory doesnt have index file
                    {
                        //403 forbidden
                        _status_code = 403;
                    }
                }
                else //request uri doesnt have "/" added at the end
                {
                    //make a 301 redirection to request uri "/" added at the end
                    //301 moved permanently
                    _status_code = 301;
                }
            }
            else//file
            {
                if (Request::if_location_has_cgi())
                {
                    //run cgi on requested file with POST REQUEST_METHOD
                    //return code depend on cgi
                    Request::run_cgi();
                }
                else //location doesnt have cgi
                {
                    //403 Forbidden
                    _status_code = 403;
                }
            }
        }
        else//not found
        {
			if (_status_code != 403)
			{
                 //404 not found
            _status_code = 404;
			}
        }
    }
}


void Request::DELETE(){
    if (Request::get_requested_resource())//found
    {
        if (Request::get_resource_type() == "directory")//directory
        {
            if(Request::is_uri_has_slash_in_end())
            {
                    if (Request::has_write_acces_on_folder())//
                    {
                        if (Request::delete_all_folder_content())//success
                        {
                            //204 no content
                            _status_code = 204;
                        }
                        else//failure
                        {
                            //500 internal server error
                            _status_code = 500;
                        }
                    }
                    else
                    {
                        //403 forbidden
                        _status_code = 403;
                    }
            }
            else //request doesnt have "/" at the end 
            {
                //409 conflict
                _status_code = 409;
            }
        }
        else //file
        {
                if (access(_requested_resource.c_str(),W_OK) ==0)
                {
                    if (remove(_requested_resource.c_str())==0)//success
                    {
                        //204 no content
                        _status_code = 204;
                    }
                    else
                    {
                        //500 internal server error
                        _status_code = 500;
                    }
                }
                else//remove failed
                {
                    //403 forbidden
                    _status_code = 403;
                }
        }
    }
    else //not found
    {
        if (_status_code != 403)
        {
                 //404 not found
            _status_code = 404;
        }
    }
}

bool Request::check_forbidden_path()
{
	char path[_requested_resource.length() + 1];
	realpath(_requested_resource.c_str() ,path);
	std::string real_path = std::string(path);
    if (real_path.length() <_sf->getServers()[_server_index]->getLocations()[_location_index]->getRoot().length())//the path is forbidden stop processing the request and return 403 forbidden 
    {
        _status_code=403;
        return false;
    }
    return true;
}


//----------helper functions
bool Request::get_requested_resource(){
//append request uri to root
    std::string rsc;
    std::string tmp = _sf->getServers()[_server_index]->getLocations()[_location_index]->getRoot();
    if (tmp.find("/")==tmp.length()-1)//remove "/" from root
        tmp = tmp.substr(0,tmp.length()-1);
    if (_sf->getServers()[_server_index]->getLocations()[_location_index]->getPath() == "/")
        rsc = tmp+_RequestURI; 
    else{
        //remove matched path from req_uri
        std::string new_rsc = _RequestURI.substr(_sf->getServers()[_server_index]->getLocations()[_location_index]->getPath().length());
        if (new_rsc.find("/")!=0)
            tmp.append("/");
        rsc = tmp+ new_rsc;
    }
	//
    _requested_resource = rsc;
    if (check_forbidden_path()== false)
        return false;
    //check if the requested resource is a file
    struct stat fileInfo;
    if (stat(rsc.c_str(),&fileInfo)!=0)
        std::cout<<"stat function: failed to get information  " << rsc  <<std::endl;
    else if (S_ISREG(fileInfo.st_mode))//regular file found
    {
        _resource_type = "file";
        _requested_resource = rsc;
        //get filename_extension for cgi
        if (_requested_resource.find(".")!=std::string::npos)
            _filename_extension= _requested_resource.substr(_requested_resource.find_last_of('.')+1);
        return true;
    }
    else if (S_ISDIR(fileInfo.st_mode))//directory found
    {
        _resource_type = "directory";
        _requested_resource = rsc;
		_filename_extension = "not a file";
		//   _filename_extension= _requested_resource.substr(_requested_resource.find_last_of('.')+1);
        return true;
    }
    _resource_type = "";
    return false;
}
std::string Request::get_resource_type(){return _resource_type;}

bool Request::is_uri_has_slash_in_end(){
    if (_RequestURI[_RequestURI.length()-1] == '/')
        return true;
    return false;
}

bool Request::indexFileExists(const char *dir_path, std::string &filename) {
	std::string locationName = _location_index;
	if(locationName[locationName.length()-1]!= '/')
		locationName.append("/");
	if(_RequestURI == locationName  && _sf->getServers()[_server_index]->getLocations()[_location_index]->getIndex() != "")
	{
		if(!_sf->getServers()[_server_index]->getLocations()[_location_index]->getIndex().empty())
		{
			filename = _sf->getServers()[_server_index]->getLocations()[_location_index]->getIndex();
			return(true);
		}
	}
	else {
		struct dirent* entry;
		DIR* directory = opendir(dir_path);
		if(directory == nullptr) {
			std::cout << "search_for_filename : cannot open directory" << std::endl;
			return false ;
		}
		while((entry = readdir(directory)) != nullptr) {
			if(strncmp(entry->d_name, "index", 5) == 0) {
				filename = std::string(entry->d_name);
				closedir(directory);
				return(true);
			}
		}
		closedir(directory);
	}
	return(false);
}

bool Request::search_for_indexfile(const char *dir_path){
    std::string filename ;
	if(indexFileExists(dir_path, filename) == true)
	{
		_resource_type = "file";
		_requested_resource += filename;
		return (true);
	}
	else
    	return false;
}

bool Request::is_dir_has_index_file(){
    return search_for_indexfile(_requested_resource.c_str());
}
std::string Request::get_auto_index(){
    return _sf->getServers()[_server_index]->getLocations()[_location_index]->getAutoIndex();
}
bool Request::if_location_has_cgi(){
	std::map<std::string, std::string> map = _sf->getServers()[_server_index]->getLocations()[_location_index]->getCgiPath();
	if(!map.empty() && (map.find(_filename_extension) != map.end())) {
		_cgi_flag = true;
		return true;
	}
	return false;
}


bool Request::if_location_support_upload(){
     if (_sf->getServers()[_server_index]->getLocations()[_location_index]->getUploadPath() == "" )
        return false;
    return true;
}

int nftwcheck(const char *filename, const struct stat *statptr, int fileflags, struct FTW *pfwt){
    (void)pfwt;   
    if ((fileflags == FTW_DP || fileflags == FTW_DNR)&& (statptr->st_mode & !S_IWOTH))//directory 
            return -1;
    else if (access(filename,W_OK)!=0)//file or other type
            return -1;
    return 0;
}
bool Request::has_write_acces_on_folder(){
    //walk the folder tree with nft and check write access
    int flags = FTW_DEPTH;
    struct stat folderStats;
    if (stat(_requested_resource.c_str(),&folderStats) ==-1 || (folderStats.st_mode & !S_IWOTH))
	{
        return false;
	}
	if (nftw(_requested_resource.c_str(), nftwcheck, 1000, flags) ==0)
	{
            return true; 
	}
    return false;
}


int nftwfunc(const char *filename, const struct stat *statptr, int fileflags, struct FTW *pfwt)
{
    (void)pfwt;
    (void)statptr;
    //delete here

    if (fileflags == FTW_DP || fileflags == FTW_DNR)//directory 
	{
		if (rmdir(filename)==-1)
        	return -1;

	}
    else if (remove(filename) == -1)//file or other type
	{
            return -1;
	}
    return 0;
}
bool Request::delete_all_folder_content(){
    int flags = FTW_DEPTH;
	if (nftw(_requested_resource.c_str(), nftwfunc, 1000, flags) ==0)
    {
        //nftw does not remove the parent folder
        // if (rmdir(_requested_resource.c_str())==-1)
            // return false;
        return true;
    }
    return false;
}

bool Request::request_is_ready(){
	if (_method == "POST")
		return _b_complete && _upload_done;
	else
		return _b_complete;
}

void Request::upload_resource(){
    //check content-type exist and = "multipart/form-data"
    if (_Headers.find("Content-Type")!= _Headers.end()&&_Headers["Content-Type"].find("multipart/form-data")!=std::string::npos)
        Request::handle_multipart_form_data();
    else{
		std::string tmp0 = _Headers["Content-Type"];
		Http::trimSpaces(tmp0);
		std::string ext  =_sf->getReverseMIMEtypes()[tmp0];
        //uploading file
        _upload_filename = _sf->getServers()[_server_index]->getLocations()[_location_index]->getUploadPath() + random_filename()+"."+ext;
        _Body.open(_filename,std::ios::in);
        _upload_file.open(_upload_filename,std::ios::out|std::ios::app);
        if (_Body.is_open() && _upload_file.is_open())
        {
            std::string line;
            while(getline(_Body,line))
                _upload_file<<line.append("\n");
            _Body.close();
            _upload_file.close();
        }
    }

}


void Request::handle_multipart_form_data(){
    //create tmp fstream with tmp filename
    std::string _tmp_filename = TMP_PATH+random_filename()+".mulipart";
    std::fstream tmp;
    std::string buffer = _Headers["Content-Type"];
    //get boundary
    std::string boundary = buffer.substr(buffer.find("boundary=")+9);
    _Body.open(_filename,std::ios::in);
    tmp.open(_tmp_filename,std::ios::out| std::ios::app);
    if (_Body.is_open() && tmp.is_open())
    {
        std::string line;
        int count =0;
        while(getline(_Body,line))
        {   
            if (line.find(boundary) !=std::string::npos)//skip boundary
            {
                if (count==0)
                    count++;
                else
                    getline(_Body,line);
            }
            else if (line.find("Content-Disposition:")!=std::string::npos)//get filename from header
            {
                    //extract filename
                    _upload_filename = line.substr(line.find("filename=")+10);
                    _upload_filename = _sf->getServers()[_server_index]->getLocations()[_location_index]->getUploadPath()+_upload_filename.substr(0,_upload_filename.find_last_of("\""));
            }
            else if (line.find("Content-Type")!= std::string::npos)//skip header
                getline(_Body,line);
            else
                tmp<<line.append("\n");
        }
        _Body.close();
        tmp.close();
    }
    //upload
    tmp.open(_tmp_filename,std::ios::in);
    _upload_file.open(_upload_filename,std::ios::out| std::ios::app);
    if (tmp.is_open() && _upload_file.is_open())
    {
        std::string line;
        while(getline(tmp,line))
            _upload_file<<line.append("\n");
        tmp.close();
        _upload_file.close();
    }
}




//CGI
//use putenv() to add env vars
void Request::set_cgi_env()
{
    std::string head;
    _script_name=_RequestURI.substr(_RequestURI.find_last_of('/')+1,_RequestURI.find("?"));
    _path_info = _RequestURI.substr(0,_RequestURI.find_last_of('/'));
    _path_translated = _script_name;
    //SERVER_SOFTWARE
    head = "SERVER_SOFTWARE=webserv";
    putenv(strdup(head.c_str()));
    head.clear();
    //SERVER_NAME
    head = "SERVER_NAME="+ _sf->getServers()[_server_index]->getServerName();
    putenv(strdup(head.c_str()));
    head.clear();
    //GATEWAY-INTERFACE
    head = "GATEWAY_INTERFACE=CGI/1.1";
    putenv(strdup(head.c_str()));
    head.clear();
    //SERVER_PROTOCOL
    head = "SERVER_PROTOCOL="+_http_v;
    putenv(strdup(head.c_str()));
    head.clear();
    //SERVER_PORT
    head ="SERVER_PORT="+ _req_port;
    putenv(strdup(head.c_str()));
    head.clear();
    //REQUEST_METHOD
    head = "REQUEST_METHOD="+_method;
    putenv(strdup(head.c_str()));
    head.clear();
    //PATH_INFO
    head = "PATH_INFO="+_path_info;
    putenv(strdup(head.c_str()));
    head.clear();
    //PATH_TRANSLATED
    head = "PATH_TRANSLATED="+_path_translated;
    putenv(strdup(head.c_str()));
    head.clear();    
    //SCRIPT_NAME
    head = "SCRIPT_NAME="+_script_name;
    putenv(strdup(head.c_str()));
    head.clear();  
    //SCRIPT_FILENAME
    head = "SCRIPT_FILENAME="+ _requested_resource;
    putenv(strdup(head.c_str()));
    head.clear();  
    //
    //DOCUMENT_URI
    head = "DOCUMENT_URI="+ _RequestURI;
    putenv(strdup(head.c_str()));
    head.clear();  
	//DOCUMENT_ROOT
	 head = "DOCUMENT_ROOT="+_sf->getServers()[_server_index]->getRoot();
    putenv(strdup(head.c_str()));
    head.clear();  
    //PATH
    head = "PATH="+ std::string(std::getenv("PATH"));  
    putenv(strdup(head.c_str()));
    head.clear(); 
    //QUERY_STRING
    head = "QUERY_STRING="+_query_string;
    putenv(strdup(head.c_str()));
    head.clear();
	//REQUEST_URI
	head = "REQUEST_URI="+_RequestURI;
    putenv(strdup(head.c_str()));
    head.clear();  
    //REMOTE_ADDR
    head = "REMOTE_ADDR="+_req_host;
    putenv(strdup(head.c_str()));
    head.clear();  
    //CONTENT-TYPE
    if (_Headers.find("Content-Type")!= _Headers.end())
    {
        std::string tmp = _Headers["Content-Type"];
        Http::trimSpaces(tmp);
        head = "CONTENT_TYPE="+tmp;
        std::cerr<<"content_type = ["<<head<<"]"<<std::endl;
    }
    else 
        head = "CONTENT_TYPE=";
    putenv(strdup(head.c_str()));
    head.clear();
    //CONTENT-LENGTH
    if (_Headers.find("Content-Length")!= _Headers.end())
    {
        std::string tmp = _Headers["Content-Length"];
        Http::trimSpaces(tmp);
        head = "CONTENT_LENGTH="+tmp;
    std::cerr<<"content_length = ["<<head<<"]"<<std::endl;
    }
    else 
        head = "CONTENT_LENGTH=";
    putenv(strdup(head.c_str()));

    head.clear();
    head = "REDIRECT_STATUS=200";
    putenv(strdup(head.c_str()));
    head.clear();    
    // HTTP_ + "to_upper<header-name>" (replace any '-' with '_')
    //debug
    std::cerr<<"-------------------start of http headers---------------"<<std::endl;
    //end debug
    for (std::map<std::string,std::string>::iterator it = _Headers.begin(); it != _Headers.end();it++)
    {
        std::string head_name= it->first;
        std::string head_value = it->second;
        Http::trimSpaces(head_value);
        for (size_t i=0;i<head_name.length();i++)
        {
            if (head_name[i] == '-')
                head_name[i] = '_';
            head_name[i] = toupper(head_name[i]);
        }
        head = "HTTP_" + head_name + "=" + head_value;
        //debug
        std::cerr<<"http result head = ["<<head.c_str()<<"]"<<std::endl;
        //end debug
        //add env var
        putenv(strdup(head.c_str()));
        head.clear();
    }
}

void Request::clean_cgi_output(){

        //clean cgi output extract just the body and put back to cgi_out_file
		//move cgi body to output file	
			//debug
			std::cerr<<"---------------start of clean_cgi_output------------"<<std::endl;
			///end debug
		_cgi_output_filename = TMP_PATH + random_filename()+"cgi_out"+".html"; 
	
		_cgi_output.open(_cgi_output_filename,std::ios::out|std::ios::app);
		if (!_cgi_output.is_open())
			std::cerr<<"failed to open cgi_outfileeeeeeee"<<std::endl;

			//debug
			std::cerr<<"cgi_out file =["<<_cgi_output_filename<<"]"<<std::endl;
			std::cerr<<"tmp_file =["<<_cgi_tmpfilename<<"]"<<std::endl;
			//end debug
		std::fstream tmp;
		tmp.open(_cgi_tmpfilename,std::ios::in);
		if (!tmp.is_open())
			std::cerr<<"failed to open tmpppppppppppp"<<std::endl;
		perror("Error ====== ");
		if (_cgi_output.is_open() && tmp.is_open())
		{
            std::string line;
            std::string	buffer;
            std::string heads;
            //set headers
       
			if (_filename_extension != "py")
			{     //debug
            std::cerr<<"first loooooooop"<<std::endl;
            //end debug
            while(getline(tmp,line))
            {
                //debug
                std::cerr<<"line = ["<<line<<"]"<<std::endl;
                //end debug
                if (line == "\r")
                    break;
                if (line.find(":")!=std::string::npos)
                        _cgi_headers.insert(std::make_pair(line.substr(0,line.find(":") + 1),line.substr(line.find(":")+1)));
            }
			}
             //debug
            std::cerr<<"second loooooooop"<<std::endl;
            //end debug
            while (getline(tmp,line))
            {
                //debug
                std::cerr<<"line = ["<<line<<"]"<<std::endl;
                //end debug
                _cgi_output<<line.append("\n");
            }
            _cgi_output.close();
            tmp.close();
            // remove(tmp_file.c_str());
        
		}
		else 
			std::cerr<<"clean cgi output: failed to open either cgi_output or tmp"<<std::endl;
		//debug
		std::cerr<<"--------------cgi headers----------"<<std::endl;
		for (std::map<std::string, std::string>::iterator it = _cgi_headers.begin();it!=_cgi_headers.end();it++)
		{
			std::cerr<<"name = ["<<it->first<<"]"<<std::endl;
			std::cerr<<"value = ["<<it->second<<"]"<<std::endl;
		}
		std::cerr<<"------------------------------------"<<std::endl;
		//end debug
}

//

//print
void debug_print_env(char** environ)
{
    //
    std::cerr<<"-----------------------------env passed to cgi----------"<<std::endl;
    int i=0;
    while(environ[i])
    {
        std::cerr<<"env = ["<<environ[i]<<"]"<<std::endl;
        i++;
    }
    std::cerr<<"----------------------------------------------------------"<<std::endl;

}

void Request::debug_cgi()
{
    std::cerr<<"-----------------------------debugging cgii start---------------------"<<std::endl;
    std::cerr<<"server_name = ["<<_sf->getServers()[_server_index]->getServerName()<<"]"<<std::endl;
    std::cerr<<"server_protocol = ["<<_http_v<<"]"<<std::endl;
    std::cerr<<"server_port = ["<<_req_port<<"]"<<std::endl;
    std::cerr<<"server_method = ["<<_method<<"]"<<std::endl;
    std::cerr<<"path_info = ["<<_path_info<<"]"<<std::endl;
    std::cerr<<"path_translated = ["<<_path_translated<<"]"<<std::endl;
    std::cerr<<"script_name = ["<<_script_name<<"]"<<std::endl;
    std::cerr<<"query_string = ["<<_query_string<<"]"<<std::endl;
    std::cerr<<"remote_host = ["<<_remote_host<<"]"<<std::endl;
    std::cerr<<"remote_addr = ["<<_remote_addr<<"]"<<std::endl;
    std::cerr<<"auth_type = ["<<_auth_type<<"]"<<std::endl;
    std::cerr<<"remote_user = ["<<_remote_user<<"]"<<std::endl;
    
    std::cerr<<"remote_ident = ["<<_remote_ident<<"]"<<std::endl;
    std::cerr<<"content_type = ["<<_Headers["Content-Type"]<<"]"<<std::endl;
    std::cerr<<"-----------------------------------------------------------------------"<<std::endl;

}


extern char** environ;//user env
void Request::run_cgi(){
    //debug
    std::cerr<<"------------------------------starting cgi-----------------------------"<<std::endl;
    //end debug
    ////after debugging remove cgi_file to tmp folder
	std::fstream tmp;
    _cgi_tmpfilename = TMP_PATH+ random_filename()+".html";
    //debug
    std::cerr<<"tmp_filename = ["<<_cgi_tmpfilename<<"]"<<std::endl;
    //end debug
   
    //debug
   

    //end debug
    //fork
    _cgi_pid = fork();
    if (_cgi_pid == -1)
        std::cout<<"run_cgi: fork function failed"<<std::endl;
    //debug
    std::cerr<<"debug_cgi : fork"<<std::endl;
    //end debug

    if (_cgi_pid ==0)
    {
		 set_cgi_env();
		  debug_cgi();
        //debug
        debug_print_env(environ);
        //end debug
        if (_method=="POST"){
			
            int in_fd = open(_filename.c_str(), O_RDWR);
            if (in_fd==-1)
                std::cout<<"run_cgi: failed to open the request body file for reading filname = ["<<&_filename[0]<<"]"<<std::endl;
			perror("Error = ");
            dup2(in_fd,0);
            close(in_fd);
        }
        int out_fd = open(_cgi_tmpfilename.c_str(), O_RDWR|O_CREAT|O_APPEND, 0644);
        if (out_fd==-1)
            std::cout<<"run_cgi: failed to open the cgi out file for writing"<<std::endl;
        dup2(out_fd,1);
        close(out_fd);
                //debug
                std::cerr<<"debug_cgi : child proccess"<<std::endl;
                //end debug

            //cgi arguments (script name +file name)
            char **args = new char*[3];
            //debug
            std::cerr<<"file_name extension = ["<<_filename_extension<<"]"<<std::endl;
            //end ddebug

            std::string cgi_bin = _sf->getServers()[_server_index]->getLocations()[_location_index]->getCgiPath()[_filename_extension];
            //debug
            std::cerr<<"cgi_bin == ["<<cgi_bin<<"]"<<std::endl;
            //end debug
            args[0] = strdup(cgi_bin.c_str());
                //debug
                std::cerr<<"debug_cgi : arg[0] == ["<<args[0]<<"]"<<std::endl;
                //end debug
            args[1] = strdup(_requested_resource.c_str());
                //debug
                std::cerr<<"debug_cgi : arg[1] == ["<<args[1]<<"]"<<std::endl;
                //end debug
            args[2] =NULL;
            execve(args[0],args,environ);
            //free the alocated memory
            //debug
            std::cerr<<"execve : failed to execute cgi"<<std::endl;
            perror("error == ");
            //end debug
            //cgi failed
    }
    else
    {
			_status_code=200;
			_cgi_start_time = time(NULL);
			while(time(NULL) - _cgi_start_time < CGI_TIMEOUT)
			{
				//wait
				  //wait for cgi to finis
				if (waitpid(_cgi_pid,NULL,WNOHANG)==_cgi_pid)
				{
					_status_code=-2;
					break;
				}
							
			}
			if(_status_code!=-2 && waitpid(_cgi_pid,NULL,WNOHANG)!=_cgi_pid)
			{
						//kill cgi process
						kill(_cgi_pid,SIGKILL);
						//debug
						std::cerr<<"**********cgi timeout********"<<std::endl;
				// 		//end debug
						_status_code = 504;//gateway time out;
			}
			else
				_status_code=200;
    }
    //AFTER CGI EXECUTION
    //remove cgi headers and everything else the response dont need
	if (_status_code==200)
		clean_cgi_output();
    std::cerr<<"------------------------------end of cgi-----------------------------"<<std::endl;

}



//getters
std::string Request::getHttp_version()const{return _http_v;}
int			Request::getServerIndex()const{return _server_index;}
int 		Request::getStatusCode() const{return _status_code;}
std::string Request::getLocationIndex()const{return _location_index;}
std::string	Request::getresourceType()const{return _resource_type;}
std::string	Request::getRequestedresource()const{return _requested_resource;}
std::string Request::getUploadFilename()const{return _upload_filename;}
std::string Request::getUploadFile()const{return _upload_filename;}
std::string Request::getFilenameExtension()const{return _filename_extension;}
void		Request::setStatusCode(int statusCode ) {_status_code = statusCode;}
std::string Request::getCgiOutputFilename()const{return(_cgi_output_filename);}
std::map<std::string, std::string> Request::getCgiHeaders()const{return(_cgi_headers);}
bool 		Request::getCgiFlag()const{return(_cgi_flag);}


// print all request attributes
void Request::print() {
	std::ostringstream out;
	out << _Body.rdbuf();
	std::cout << "body : " << out.str() << RESET << std::endl;
	std::cout << GREEN << "method : " << _method << std::endl;
	std::cout << "Request URI : " << _RequestURI << std::endl;
	std::cout << "http_v : " << _http_v << std::endl;
	std::cout << "status code : " << _status_code << std::endl;
	std::cout << "server index : " << _server_index << std::endl;
	std::cout << "location index : " << _location_index << std::endl;
	std::cout << "req host : " << _req_host << std::endl;
	std::cout << "req port : " << _req_port << std::endl;
	std::cout << "resource type : " << _resource_type << std::endl;
	std::cout << "requested resource : " << _requested_resource << std::endl;
	std::cout << "====== for cgi ======" << std::endl; 
	std::cout << "cgi  output filename : " << _cgi_output_filename << std::endl;
	std::cout << "cgi _flag : " << _cgi_flag << RESET <<std::endl;
}