#include "Request.hpp"



Request::Request() {}
Request::Request(std::string request_host, std::string request_port):_sf(ServerFarm::getInstance()),_req_host(request_host),_req_port(request_port){
	_method = "";
	_RequestURI = "";
	_http_v = "";
	_status_code = -1;
	_server_index = -1;
	_location_index = -1;
	_resource_type = "";
	_requested_resource = "";
	_upload_filename = "";
	_filename_extension = "";
}

Request::Request(const Request& other) {
	(void)other;
}

Request& Request::operator=(const Request& other) {
	(void)other;
	return(*this);
}

Request::~Request() {}


void Request::proccess_Request(std::string req_data){
    _bytes_read = req_data.length();
    _message+=req_data;
    parse();
    //initial status code (if status code remain -1 that means no errors found at this stage)
    //if its not a valid httpmessage stop here
    if (request_is_ready())
    {
            //debug
            //print();
            //end debug
        _status_code=-1;
        _server_index =-1;
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
                std::cout<<"error: no server found to handle the request"<<std::endl;
        }
        std::istringstream iss(_startLine);
        iss>>_method;
        iss>>_RequestURI;
        iss>>_http_v;
        //debug
        // std::cerr<<"Request: _method= "<<_method<<" _uri= "<<_RequestURI<<" http= "<<_http_v<<std::endl;
        //end debug
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
        else if (_RequestURI.length() > 2097152)//chrome max uri size
        {
            //414 Request-URI Too Long
            _status_code = 414;
        }
        else if ((_sf->getServers())[_server_index]->getClientBodySizeLimit() < _body_length)//request body larger than client max body size in config file
        {
            //413 Request Entity Too Large
            _status_code  = 413;
        }
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
                if (Request::is_method_allowed_in_location())
                    check_which_requested_method();
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

std::string Request::getMethod()const{return _method;}
std::string Request::getRequestURI()const{return _RequestURI;}

bool Request::check_for_forbidden_chars(std::string uri)const{
    std::string forbidden = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    for(size_t i=0;i<uri.length();i++)
    {
        if (forbidden.find(uri[i])==std::string::npos)
            return 1;
    }
    return 0;
}

void Request::get_matched_location_for_request_uri(){
	std::map<std::string, Location *> tmp;
    //loop the location
    for (std::map<std::string,Location*>::iterator it = _sf->getServers()[_server_index]->getLocations().begin();it != _sf->getServers()[_server_index]->getLocations().end(); it++)
    {
        //if you found a match return add the location to the map
        if (_RequestURI.find(it->first)==0)
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
		return(true);
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
    //debug
    std::cerr<<"Request::check_which_requested_method = [ "<<_method<<" ]"<<std::endl;
    //end debug
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
					// std::cout << RED << "am hr" << RESET << std::endl;
                    if (Request::if_location_has_cgi())//location has cgi
                    {
                        //run cgi on requested file with GET REQUEST_METHOD
                        //return code Depend on cgi 
                        //Request::run_cgi();
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
                // Request::run_cgi();
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
        if (_status_code == 403)
        {
            //403 forbidden
        }
        else
        {
            //404 not found
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
                            // Request::run_cgi();
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
                    // Request::run_cgi();
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
            if (_status_code == 403)
            {
                //403 forbidden
            }
            else
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
                if (Request::delete_all_folder_content())//success
                {
                    //204 no content
                    _status_code = 204;
                }
                else //failure
                {
                    if (Request::has_write_acces_on_folder())//
                    {
                        //500 internal server error
                        _status_code = 500;
                    }
                    else
                    {
                        //403 forbidden
                        _status_code = 403;
                    }
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
                if (remove(_requested_resource.c_str())==0)//success
                {
                    //204 no content
                    _status_code = 204;
                }
                else//remove failed
                {
                    if (access(_requested_resource.c_str(),W_OK) ==0)
                    {
                        // //500 internal server error
                        _status_code = 500;
                    }
                    else
                    {
                        //403 forbidden
                        _status_code = 403;
                    }
                }
        }
    }
    else //not found
    {
        if (_status_code == 403)
        {
            //403 forbidden
        }
        else
        {
            //404 not found
            _status_code = 404;
        }
    }
}

bool Request::check_forbidden_path()
{
    std::string real_path = realpath(_requested_resource.c_str(),0);
    if (real_path.length() <_sf->getServers()[_server_index]->getLocations()[_location_index]->getRoot().length())//the path is forbidden stop processing the request and return 403 forbidden 
    {
        //denbug
        std::cerr<<"found forbidden path == ["<<_requested_resource<<"]"<<std::endl;
        //end debug
        _status_code=403;
        return false;
    }
    //        //denbug
        std::cerr<<"found allowed path == ["<<_requested_resource<<"]"<<std::endl;
        //end debug

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
    _requested_resource = rsc;
    //debug
    std::cerr<<"Request::get_requested_resource = ["<<_requested_resource<<"]"<<std::endl;
    //end debug
    if (check_forbidden_path()== false)
        return false;
    //check if the requested resource is a file
    struct stat fileInfo;
    if (stat(rsc.c_str(),&fileInfo)!=0)
        std::cout<<"stat function: failed to get information"<<std::endl;
    else if (S_ISREG(fileInfo.st_mode))//regular file found
    {
        _resource_type = "file";
        _requested_resource = rsc;
        return true;
    }
    else if (S_ISDIR(fileInfo.st_mode))//directory found
    {
        _resource_type = "directory";
        _requested_resource = rsc;
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

//helper function
bool search_for_indexfile(const char *dir_path){
    struct dirent* entry;
    std::string filename = "index"; 
    DIR* directory =opendir(dir_path);
    if (directory == nullptr)
    {
        std::cout<<"search_for_filename: error cant open directory"<<std::endl;
        return false;
    }
    while ((entry = readdir(directory)) != nullptr)
    {
        if (strncmp(entry->d_name,filename.c_str(),5)==0)
        {
            closedir(directory);
            return true;
        }
    }
    closedir(directory);
    return false;
}

bool Request::is_dir_has_index_file(){
    return search_for_indexfile(_requested_resource.c_str());
}
std::string Request::get_auto_index(){
    return _sf->getServers()[_server_index]->getLocations()[_location_index]->getAutoIndex();
}
bool Request::if_location_has_cgi(){
    if (_sf->getServers()[_server_index]->getLocations()[_location_index]->getCgiPath().empty()
    || (_sf->getServers()[_server_index]->getLocations()[_location_index]->getCgiPath().find(_filename_extension) == _sf->getServers()[_server_index]->getLocations()[_location_index]->getCgiPath().end()))
        return false;
    return true;
}
bool Request::if_location_support_upload(){
     if (_sf->getServers()[_server_index]->getLocations()[_location_index]->getUploadPath() == "" )
        return false;
    return true;
}
bool Request::has_write_acces_on_folder(){
    struct stat folderStats;
    if (stat(_requested_resource.c_str(),&folderStats) ==-1)
        return false;
    if (S_ISDIR(folderStats.st_mode) && (folderStats.st_mode & S_IWOTH))
        return true; 
    return false;
}


int nftwfunc(const char *filename, const struct stat *statptr, int fileflags, struct FTW *pfwt)
{
    (void)pfwt;
    (void)statptr;
    //delete here
    if (fileflags == FTW_SL)//symbolik link
    {
        if (unlink(filename)==-1)
            return -1;
    }
    else if (fileflags == FTW_DP|| fileflags == FTW_DNR)//directory 
    {
        //FTW_DNR
        // The object is a directory that cannot be read. The fn function shall not be called for any of its descendants.
        //FTW_DP
        // The object is a directory and subdirectories have been visited. (This condition shall only occur if the FTW_DEPTH flag is included in flags.)
        if (rmdir(filename)==-1)
            return -1;
    }
    else//file or other type
    {
        if (remove(filename) == -1)
            return -1;
    }
    return 0;
}
bool Request::delete_all_folder_content(){
    int flags = FTW_DEPTH;
	if (nftw(_requested_resource.c_str(), nftwfunc, 100, flags) ==0)
    {
        //nftw does not remove the parent folder
        if (rmdir(_requested_resource.c_str())==-1)
            return false;
        return true;
    }
    return false;
}

bool Request::request_is_ready(){
    //debug
    if (_b_complete)
        std::cerr<<"Request;;request_is_ready"<<std::endl;
    //end debug
    return _b_complete;
}

void Request::upload_resource(){
    //check content-type exist and = "multipart/form-data"
    if (_Headers.find("Content-Type")!= _Headers.end())
    {
        //debug
        std::cerr<<"Request::upload_resource: about to upload"<<std::endl;
        //end debug
        std::string line  = _Headers["Content-Type"];
        if (line.find("multipart/form-data")!=std::string::npos)
            Request::handle_multipart_form_data();
    }
    else{
        _upload_filename = random_filename();
        //uploading file
        _upload_filename = _sf->getServers()[_server_index]->getLocations()[_location_index]->getUploadPath() + _upload_filename;
        _Body.open(_filename,std::ios::in);
        _upload_file.open(_upload_filename,std::ios::out|std::ios::app);
        if (_Body.is_open() && _upload_file.is_open())
        {
            std::string line;
            while(getline(_Body,line))
                _upload_file<<line.append("/");
            _Body.close();
            _upload_file.close();
        }
    }
}


void Request::handle_multipart_form_data(){
    //debug     
    std::cerr<<"Request::handling multipart inside the function"<<std::endl;
    //end debug
    //create tmp fstream with tmp filename
    std::string tmp_filename = "/Users/hassan/Desktop/request2.0/tmp/"+random_filename()+".mulipart";
    std::fstream tmp;
    std::string buffer = _Headers["Content-Type"];
    //get boundary
    std::string boundary = buffer.substr(buffer.find("boundary=")+9);
    _Body.open(_filename,std::ios::in);
    tmp.open(tmp_filename,std::ios::out| std::ios::app);
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
                    //debug
                    std::cerr<<"upload_filename = ["<<_upload_filename<<"]"<<std::endl;
                    //end debug
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
    tmp.open(tmp_filename,std::ios::in);
    _upload_file.open(_upload_filename,std::ios::out| std::ios::app);
    if (tmp.is_open() && _upload_file.is_open())
    {
        //debug
        std::cerr<<"----------------uploadfile is open for appending-----------"<<std::endl;
        //end debug
        std::string line;
        while(getline(tmp,line))
            _upload_file<<line.append("\n");
        tmp.close();
        _upload_file.close();
    }
}




//this the code below is for cgi
void Request::prepare_env(){

    //prepare the env vars that you dont have already
    //PATH_INFO
    _path_info = _RequestURI.substr(0,_RequestURI.find("?"));
    if (!_path_info.empty())
    {
        //PATH_TRANSLATED
        //if PATH_INFO is not set you should set the var too
    }
    //SCRIPT_NAME
    _script_name = _RequestURI.substr(0,_RequestURI.find("?"));
    //QUERY_STRING
    if (_RequestURI.find("?")!=std::string::npos)
        _query_string = _RequestURI.substr(_RequestURI.find("?")+1);
    //REMOTE_HOST
    //REMOTE_ADDR
    //AUTH_TYPE
    //REMOTE_USER
    //REMOTE_IDENT
}

//use putenv() to add env vars
void Request::set_cgi_env()
{
    std::string head;
    //SERVER_SOFTWARE???
    //SERVER_NAME
    head = "SERVER_NAME="+ _sf->getServers()[_server_index]->getServerName();
    putenv(&head[0]);
    head.clear();
    //GATEWAY-INTERFACE???
   
    //SERVER_PROTOCOL
    head = "SERVER_PROTOCOL="+_http_v;
    putenv(&head[0]);
    head.clear();
    //SERVER_PORT
    head ="SERVER_PORT="+ _req_port;
    putenv(&head[0]);
    head.clear();
    //REQUEST_METHOD
    head = "REQUEST_METHOD="+_method;
    putenv(&head[0]);
    head.clear();
    //PATH_INFO
    head = "PATH_INFO="+_path_info;
    putenv(&head[0]);
    head.clear();
    //PATH_TRANSLATED
    head = "PATH_TRANSLATED="+_path_translated;
    putenv(&head[0]);
    head.clear();    
    //SCRIPT_NAME
    head = "SCRIPT_NAME="+_script_name;
    putenv(&head[0]);
    head.clear();    
    //QUERY_STRING
    head = "QUERY_STRING="+_query_string;
    putenv(&head[0]);
    head.clear();    
    //REMOTE_HOST
    head = "REMOTE_HOST="+_remote_host;
    putenv(&head[0]);
    head.clear();    
    //REMOTE_ADDR
    head = "REMOTE_ADDR="+_remote_addr;
    putenv(&head[0]);
    head.clear();    
    //AUTH_TYPE
    head = "AUTH_TYPE="+_auth_type;
    putenv(&head[0]);
    head.clear();    
    //REMOTE_USER
    head = "REMOTE_USER="+_remote_user;
    putenv(&head[0]);
    head.clear();    
    //REMOTE_IDENT
    head = "REMOTE_IDENT="+_remote_ident;
    putenv(&head[0]);
    head.clear();    
    //CONTENT-TYPE
    if (_Headers.find("Content-Type")!= _Headers.end())
        head = "CONTENT_TYPE="+_Headers["Content-type"];
    else 
        head = "CONTENT_TYPE=";
    putenv(&head[0]);
    head.clear();
    //CONTENT-LENGTH
    if (_Headers.find("Content-Length")!= _Headers.end())
        head = "CONTENT_LENGTH="+_Headers["Content-Length"];
    else 
        head = "CONTENT_LENGTH=";
    putenv(&head[0]);
    head.clear();    
    // HTTP_ + "to_upper<header-name>" (replace any '-' with '_')
    for (std::map<std::string,std::string>::iterator it = _Headers.begin(); it != _Headers.end();it++)
    {
        std::string head_name= it->first;
        for (size_t i=0;i<head_name.length();i++)
        {
            if (head_name[i] == '-')
                head_name = '_';
        }
        std::transform(head_name.begin(), head_name.end(), head_name.begin(), ::toupper);//transform every char in head_name to upper case
        head = "HTTP_" + head_name + "=" + it->second;
        //add env var
        putenv(&head[0]);
        head.clear();
    }
}

void Request::clean_cgi_output(){
        //clean cgi output extract just the body and put back to cgi_out_file
}


extern char** environ;//user env
void Request::run_cgi(){
    _cgi_out_filename = random_filename();
    prepare_env();
    set_cgi_env();
    //what check should i perform on cgi scritp ????????? ex:for infinite loop.....
    int in_fd = open(&_filename[0],O_RDONLY);
    if (in_fd==-1)
        std::cout<<"run_cgi:failed to open the request body file for reading"<<std::endl;
    dup2(0,in_fd);
    close(0);
    int out_fd = open(&_cgi_out_filename[0],O_WRONLY);
    if (out_fd==-1)
        std::cout<<"run_cgi:failed to open the cgi out file for writing"<<std::endl;
    dup2(1,out_fd);
    close(1);
    //fork
    pid_t child_pid = fork();
    if (child_pid == -1)
        std::cout<<"run_cgi: fork function failed"<<std::endl;
    else if (child_pid ==0)
    {
            //cgi arguments (script name +file name)
            char **args = new char*[3];
            args[0] = &_script_name[0];
            args[1] = &_filename[0];
            args[3] =NULL;
            //add signals to kill the cgi proccess if it takes too long for cgi to finish
            execve(&(_sf->getServers()[_server_index]->getLocations()[_location_index]->getCgiPath()[_filename_extension][0]),args,environ);
    }
    else
    {
        //wait for cgi to finish
        //check if any flags are needed
        waitpid(child_pid,NULL,0);
        close(in_fd);
        close(out_fd);
    }
    //AFTER CGI EXECUTION

    //print the cgi output and debugg
    //debug
    _cgi_out_file.open(_cgi_out_filename,std::ios::in);
    if (_cgi_out_file.is_open())
    {
        std::cout<<"---------------------------| this is cgi output |------------------------------------"<<std::endl;
        std::string line;
        while(getline(_cgi_out_file,line)){
            std::cout<<line<<std::endl;
        }
        std::cout<<"---------------------------| end of cgi output |------------------------------------"<<std::endl;
        _cgi_out_file.close();
    }
    //end debug

    //remove cgi headers and everything else the response dont need
    clean_cgi_output();
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


// print all request attributes
void Request::print() {
  std::string logfilename="/Users/hassan/Desktop/request2.0/tmp/log_file";
    std::fstream logfile;
    logfile.open(logfilename,std::ios::out);
    if (logfile.is_open())
    {
        
        std::cout << "__LOGFILE__DBG__ : " << std::endl;
        //redirect output to a logfile
        logfile << CYAN << "Http message class : " << std::endl;
        logfile << "startline : " << _startLine << std::endl;
        logfile << "Headers : " << std::endl;
        std::map<std::string, std::string>::iterator It;
        for(It = _Headers.begin(); It != _Headers.end(); It++) {
            logfile << "key : " << It->first << std::endl;
            logfile << "value : " << It->second << std::endl;
        }
        _Body.open(_filename,std::ios::in);
        logfile << "body : ";
        std::string line;
        while(getline(_Body,line))
            logfile<<line.append("/n");
        logfile << GREEN << "method : " << _method << std::endl;
        logfile << "Request URI : " << _RequestURI << std::endl;
        logfile << "http_v : " << _http_v << std::endl;
        logfile << "status code : " << _status_code << std::endl;
        logfile << "server index : " << _server_index << std::endl;
        logfile << "location index : " << _location_index << std::endl;
        logfile << "req host : " << _req_host << std::endl;
        logfile << "req port : " << _req_port << std::endl;
        logfile << "ressource type : " << _resource_type << std::endl;
        logfile << "requested ressource : " << _requested_resource << std::endl;
        logfile << "====== for POST METHOD ======" << std::endl; 
        logfile << "upload filename : " << _upload_filename << std::endl;
        logfile << "upload file : "; 
        _upload_file.open(_upload_filename,std::ios::in);
        line.clear();
        while(getline(_upload_file,line))
            logfile <<line.append("/");
        logfile << "filename extension : " << _filename_extension << RESET << std::endl;
        logfile<<"-------------------------------------------------------------------------"<<std::endl;
        _Body.close();
        _upload_file.close();
        logfile.close();
    }
    else{
        std::cerr<<"Request::print : error cant open logfile"<<std::endl;
    }
}