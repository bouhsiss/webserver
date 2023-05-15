#include "Request.hpp"



Request::Request() {}
Request::Request(std::string request_host, std::string request_port):_sf(ServerFarm::getInstance()),_req_host(request_host),_req_port(request_port){

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
    _message+=req_data;
    parse();
    //initial status code (if status code remain -1 that means no errors found at this stage)
    //if its not a valid httpmessage stop here
    if (request_is_ready())
    {
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
            
            std::map<int, Server> valid_listen_directive;
            std::map<int, Server>::iterator It;
            for(size_t i = 0; i < _sf->getServers().size(); i++) {
                if ((_sf->getServers())[i].getHost() == _req_host && (_sf->getServers())[i].getPort() == _req_port) {
                    valid_listen_directive.insert(std::make_pair(i, _sf->getServers()[i]));
                    _server_index = i;
                }
            }
            if(valid_listen_directive.size() != 1) {
                for(It = valid_listen_directive.begin(); It != valid_listen_directive.end(); It++) {
                    //test the host request header against the server_name entries of the server blocks that matched ip/port
                        if(It->second.getServerName() == _Headers["Host"])
                        {
                            _server_index = It->first;		
                            break;
                        }
                }
            }
            if(_sf->getServers()[_server_index].getServerName() != _Headers["Host"]) {
                //if you didnt find any match pass the request to the default server for ip/port (the first one)
                _server_index = valid_listen_directive.begin()->first;
            }
            else
                std::cout<<"error: no server found to handle the request"<<std::endl;
        }
        std::istringstream iss(_StartLine);
        std::string m;
        std::string r_uri;
        std::string http_v;
        iss>>m>>r_uri>>http_v;
        if (m != "GET" && m != "DELETE" && m != "POST")
        {
            //RETURN 501 (Not Implemented)
            _status_code = 501;
        }
        _method = m;
        //check http version should be 1.1 does this check matter?
        if (http_v != "HTTP/1.1")
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
        else if (Request::check_for_forbidden_chars(r_uri))//request uri contains a char not allowed
        {
            //400 Bad Request
            _status_code = 400;
        }
        else if (r_uri.length() > 2097152)//chrome max uri size
        {
            //414 Request-URI Too Long
            _status_code = 414;
        }
        else if ((_sf->getServers())[_server_index].getClientBodySizeLimit() < _body_length)//request body larger than client max body size in config file
        {
            //413 Request Entity Too Large
            _status_code  = 413;
        }
        _RequestURI = r_uri;

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
	std::cout << "before" << std::endl;
	std::cout << YELLOW << _sf->getServers()[_server_index] << RESET << std::endl;
	std::map<std::string, Location *> tmp;
    //loop the location
    for (std::map<std::string,Location*>::iterator it = _sf->getServers()[_server_index].getLocations().begin();it != _sf->getServers()[_server_index].getLocations().end(); it++)
    {
        //if you found a match return add the location to the map
        if (_RequestURI.find(it->first)==0)
		{
            tmp.insert(std::make_pair(it->first,it->second));
		}
    }
	std::cout << "after" << std::endl;
    //get the longest match
    if (tmp.size()!=0)
    {
        _location_index = "";
        for (std::map<std::string,Location*>::iterator it = tmp.begin();it!=tmp.end();it++)
        {
			std::cout << RED << "inserted" << std::endl;
            if (_location_index.size() < it->first.size())
			{
                _location_index = it->first;
			}
        }
    }
    else
    {
        std::cout<<"error: no location found to handle the request"<<std::endl;
        _location_index = "";
    }
}
bool Request::is_location_has_redirection(){
	Location *location = _sf->getServers()[_server_index].getLocations()[_location_index];
	if(location->getRedirect() != "")
		return(true);
    return false;
}
bool Request::is_method_allowed_in_location(){
	Location *location = _sf->getServers()[_server_index].getLocations()[_location_index];
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
        //404 Not Found
        _status_code = 404;
    }
}


void Request::POST(){
    //dont forget chunked request
    if (Request::if_location_support_upload())//location support upload
    {
        if (_Headers.find("Transfer-Encoding")!=_Headers.end())
        {
            //decode chunks annd upload them to the new file
            //where location??
        }
        else
        {
            //just upload the body
            //where location??
        }
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
            //return 404 Not Found
            _status_code = 404;
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
                if (Request::if_location_has_cgi())
                {
                    if (Request::is_dir_has_index_file())
                    {
                        //run cgi on requested file with DELETE REQUEST_METHOD
                        //return code depend on cgi
                        Request::run_cgi();
                    }
                    else
                    {
                        //403 forbidden
                        _status_code = 403;
                    }
                }
                else //location doesnt have cgi
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
            }
            else //request doesnt have "/" at the end 
            {
                //409 conflict
                _status_code = 409;
            }
        }
        else //file
        {
            if (Request::if_location_has_cgi())
            {
                //return code depend on cgi
                Request::run_cgi();
            }
            else
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
    }
    else //not found
    {
        //404 not found
        _status_code = 404;
    }
}


//----------helper functions
bool Request::get_requested_resource(){
    //append request uri to root
    std::string rsc;
    if (_sf->getServers()[_server_index].getLocations()[_location_index]->getPath() == "/")
        rsc = _sf->getServers()[_server_index].getLocations()[_location_index]->getRoot()+_RequestURI; 
    else{
        //remove matched path from req_uri
        std::string path=_sf->getServers()[_server_index].getLocations()[_location_index]->getPath();
        rsc = _sf->getServers()[_server_index].getLocations()[_location_index]->getRoot()+ _RequestURI.substr(path.length());
    }
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
    if (_RequestURI.find("/") == _RequestURI.length()-1)
        return true;
    return false;
}
bool Request::is_dir_has_index_file(){
    if(_sf->getServers()[_server_index].getLocations()[_location_index]->getIndex() == "" )
        return false ;
    return true;
}
std::string Request::get_auto_index(){
    return _sf->getServers()[_server_index].getLocations()[_location_index]->getAutoIndex();
}
bool Request::if_location_has_cgi(){
    if (_sf->getServers()[_server_index].getLocations()[_location_index]->getCgiPath() == "")
        return false;
    return true;
}
bool Request::if_location_support_upload(){
     if (_sf->getServers()[_server_index].getLocations()[_location_index]->getUploadPath() == "")
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

void Request::run_cgi(){
    //fork and execve cgi path with arguments
}

bool Request::request_is_ready(){
    return _b_complete;
}