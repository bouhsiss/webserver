#include "Response.hpp"

Response::Response(Request &request, int writeSock): _request(request), _writeSocket(writeSock) {
	// initializeStatusCodeMap();
	// _totalBytesSent = 0;
	// _isResponseSent = -1;
	// _fileSize = 0;
}

Request& Response::getRequest() {return(_request);}

// void Response::setContentLength(std::string filename) {
// 	std::ifstream _file(filname);
// 	_file.seekg(0, std::ios::end);
// 	std::ostream fileSize = _file.tellg();
// 	_file.seekg(0, std::ios::beg);
// 	// should add something when the file fails to open dues to permissions...
// }

// void Response::sendResponseBody(std::string filename) {
// 	char buffer[RESPONSE_BUFFER_SIZE];
// 	file.read(buffer, RESPONSE_BUFFER_SIZE);
// 	size_t bytesSent = send(_writeSocket, buffer, file.gcount());
// 	// if send failed -> drop client
// 	_totalBytesSent += bytesSent;
// }

// void Response::generateDirectoryListing(std::string filename) {
// 	(void)filename;
// }

// void Response::responseClass200() {
// 	if(_request.getMethod() == "GET") {
// 		if(_request.getresourceType() == "file")
// 			sendResponseFile(_request.getRequestedresource());
// 		else
// 			generateDirectoryListing(_request.getRequestedresource());
// 	}
// 	/*
// 	check method
// 	if(get)
// 		check if requested resource is file
// 			if yes return requested file
// 			if no generate autoindex
// 	if (post)
// 		upload the post request body
// 	if(delete) 
// 		delete success 
// 	*/
// }

// void Response::responseClass300(){

// }

// void Response::responseClass400(){}

// void Response::responseClass500(){}

// void Response::setstartLine() {
// 	_startLine = "HTTP/1.1 " + std::to_string(_statusCode) + _statusCodeMap[_statusCode];
// }

// void Response::setHeaders() {}

// void Response::sendResponse() {
// 	(void)_writeSocket;
// 	_statusCode = _request.getStatusCode();
// 	setstartLine();
// 	setHeaders();
// 	if(_statusCode >= 200 && _statusCode < 300)
// 		responseClass200();
// 	else if(_statusCode >= 300 && _statusCode < 400)
// 		responseClass300();
// 	else if(_statusCode >= 400 && _statusCode < 500)
// 		responseClass400();
// 	else if(_statusCode >= 500)
// 		responseClass500();
// }

	
// void Response::initializeStatusCodeMap() {
// 	_statusCodeMap.insert(std::make_pair(301,  "Moved Permanently"));
// 	_statusCodeMap.insert(std::make_pair(400,  "Bad request"));
// 	_statusCodeMap.insert(std::make_pair(403,  "Forbidden"));
// 	_statusCodeMap.insert(std::make_pair(404,  "Not Found"));
// 	_statusCodeMap.insert(std::make_pair(405,  "Method not Allowed"));
// 	_statusCodeMap.insert(std::make_pair(409,  "Conflict"));
// 	_statusCodeMap.insert(std::make_pair(413,  "Request Entity Too Large"));
// 	_statusCodeMap.insert(std::make_pair(414,  "Request-URI Too Long"));
// 	_statusCodeMap.insert(std::make_pair(500,  "Internal Server Error"));
// 	_statusCodeMap.insert(std::make_pair(501,  "Not Implemented"));
// 	_statusCodeMap.insert(std::make_pair(200,  "OK"));
// 	_statusCodeMap.insert(std::make_pair(201,  "Created"));
// 	_statusCodeMap.insert(std::make_pair(204,  "No Content"));
// }