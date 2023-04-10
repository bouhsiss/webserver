#include "Location.hpp"

//***===  getters  ===***
const std::string Location::getPath() {return(_path);}
const std::string Location::getAllowedMethods() {return(_allowed_methods);}
const std::string Location::getRoot() {return(_root);}
const std::string Location::getIndex() {return(_index);}
const std::string Location::getRedirect() {return(_redirect);}
const std::string Location::getAutoIndex() {return(_autoindex);}
const std::string Location::getDefaultFile() {return(_default_file);}
const std::string Location::getUploadPath() {return(_upload_path);}
const std::string Location::getCgiExtension() {return(_cgi_extension);}
const std::string Location::getCgiPath() {return(_cgi_path);}

//***===  setters  ===***
void Location::setPath(std::string path) {_path = path;}
void Location::setAllowedMethods(std::string allowedMethods) {_allowed_methods = allowedMethods;}
void Location::setRoot(std::string root) {_root = root;}
void Location::setIndex(std::string index) {_index = index;}
void Location::setRedirect(std::string redirect) {_redirect = redirect;}
void Location::setAutoIndex(std::string autoIndex) {_autoindex = autoIndex;}
void Location::setDefaultFile(std::string defaultFile) {_default_file = defaultFile;}
void Location::setUploadPath(std::string uploadPath) {_upload_path = uploadPath;}
void Location::setCgiExtension(std::string cgiExtension) {_cgi_extension = cgiExtension;}
void Location::setCgiPath(std::string cgiPath) {_cgi_path = cgiPath;}