#include "Request.hpp"

Request::Request() {}

Request::Request(const Request& other) {}

Request& Request::operator=(const Request& other) {}

Request::~Request() {}

Request::Request(std::string req_data, std::string request_host, int request_port):HttpMessage(req_data),_sf(ServerFarm::getInstance()),_req_host(request_host),_req_port(request_port){
    //initial status code (if status code remain -1 that means no errors found at this stage)
    //if its not a valid httpmessage stop here
    request_status = false;
    _status_code=-1;
    _server_index =-1
    //check if host header field
    if (_Headers.find("Host")==_Headers.end())//there is not a host header
    {
        //400 bad request
        _status_code = 400;
    }
    else //choosing the right server to handle the request
    {
        //test the port and ip add of the request against the listen dirictives of the server blocks
        
        std::vector<Server> valid_listen_dirictives;
        for (size_t i=0;i < (_sf.getServers()).size();i++)
        {
            if ((_sf.getServers())[i].getHost() == _req_host && (_sf.getServers())[i].getPort() == _req_port)
            {
                valid_listen_dirictives.push_back((_sf.getServers())[i])
            }
        }
        if(valid_listen_dirictives.size() == 1)
        {
            //choose the server to handle the request
            for (size_t i =0;i<(_sf.getServers()).size();i++)
            {
                if (valid_listen_dirictives[0] == (_sf.getServers())[i])
                {
                    _server_index = i;
                    break;
                }
            }
        }
        else 
        {
            for (size_t i=0;i<valid_listen_dirictives.size();i++){
                //test the host request header against the server_name entries of the server blocks that matched ip/port
                if (valid_listen_dirictives[i].getServerName() == _Headers["Host"])
                {
                    //choose this server(host header match the exact server_name)
                    for (size_t j=0;j<(_sf.getServers())size();j++){
                        if (valid_listen_dirictives[i] == (_sf.getServers())[j])
                        {
                            _server_index = j;
                            break;
                        }
                    }
                    break;
                }
            }
            if (_server_index == -1)
            {
                //if you didnt find any match pass the request to the default server for ip/port
                for (size_t i=0;i<(_sf.getServers()).size();i++)
                {
                    if (valid_listen_dirictives.size() && valid_listen_dirictives[0]==_(_sf.getServers())[i])
                        _server_idex=i;
                }
            }
        }
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
    else if ((_sf.getServers())[_server_index].getClientBodySizeLimit() < _Body.length())//request body larger than client max body size in config file
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
        else{
            if (Request::is_method_allowed_in_location())
                Request::check_which_requested_method();
            else{
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
    //loop the location
    for (std::map<std::string,Location*>::iterator it = (_sf.getServers())[_server_idex].getLocations().begin();it!=(_sf.getServers())[_server_idex].getLocations().end();it++)
    {
        //if you found a match return location name
        if (_RequestURI.find(it->first)==0)
        {
            _location_index =  it->first;
            return ;
        }
    }
    _location_index =  "";
}
bool Request::is_location_has_redirection(){
    if (((_sf.getServers())[_server_idex].getLocations())[_location_index].getRedirect()!="")
        return true;
    return false;
}
bool Request::is_method_allowed_in_location(){
    for (size_t i=0;i<((_sf.getServers())[_server_idex].getLocations())[_location_index].getAllowedMethods().size();i++)
    {
        if (_method == ((_sf.getServers())[_server_idex].getLocations())[_location_index].getAllowedMethods()[i])
            return true;
    }
    return false;
}
void Request::check_which_requested_method(){
    if (_method == "GET")
        GET();
    else if (_method == "POST")//check if the body arrived fully before sending response else dont call delete until then manualy?????
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
        //uplaod the post request body
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
                    Request::delete_all_folder_content();
                    if ()//success
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
                //delete the file
                //if you cant ?
                //204 no content
                _status_code = 204;
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
    if (_sf.getServers()[_server_index].getLocations()[_location_index].getPath() == "/")
        rsc = _sf.getServers()[_server_index].getLocations()[_location_index].getRoot()+_RequestURI; 
    else{
        //remove matched path from req_uri
        std::string path=_sf.getServers()[_server_index].getLocations()[_location_index].getPath();
        rsc = _sf.getServers()[_server_index].getLocations()[_location_index].getRoot().+ _RequestURI.substr(path.length());
    }
    //check if the requested resource is a file
    if (access(rcs.c_str(),R_OK) ==0)//file found
    {
        _resource_type = "file";
        _requested_resource = rsc;
        return true;
    }
    DIR* dir = opendir(rsc.c_str());
    if (dir != NULL)//directory found
    {
        closedir(dir);
        _requested_resource = rsc;
        _resource_type = "directory";
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
    if(_sf.getServers()[_server_index].getLocations()[_location_index].getIndex() == "" )
        return false ;
    return true;
}
std::string Request::get_auto_index(){
    return _sf.getServers()[_server_index].getLocations()[_location_index].getAutoIndex();
}
bool Request::if_location_has_cgi(){
    if (_sf.getServers()[_server_index].getLocations()[_location_index].getCgiPath() == "")
        return false;
    return true;
}
bool Request::if_location_support_upload(){
     if (_sf.getServers()[_server_index].getLocations()[_location_index].getUploadPath() == "")
        return false;
    return true;
}
bool Request::has_write_acces_on_folder(){
    struct stat folderStats;
    if (stat(_requested_resource.c_str(),&folderStats) ==-1)
        return false;
    if (S_ISDIR(folderStats.st_mode) && (folderStats.st_mode && S_IWOTH))
        return true; 
    return false;
}
void Request::delete_all_folder_content(){
    //how to delete ????? the cleanest way possible
}
void Request::run_cgi(){
    //fork and execve cgi path with arguments
}

void Request::add_chunk(std::string chunk){
    //add chunk to body
    //incriment the currunt_request_length
    //check if you reached the value in the header set request_status=true
}