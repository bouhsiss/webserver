#include "Response.hpp"

Response::Response(Request &request, int writeSock): _request(request), _writeSocket(writeSock) {
	initializeStatusCodeMap();
	_totalBytesSent = 0;
	_isResponseSent = -1;
	_contentLength = 0;
	_isResponseSent = false;
	_headersAreSent = false;
	_sendFailed = false;
	_statusCode = _request.getStatusCode();
}

Request& Response::getRequest() {return(_request);}

bool Response::sendFailed(){return(_sendFailed);}
bool Response::isResponseSent(){return(_isResponseSent);}

void Response::setContentLength(std::string filename) {
	_file.open(filename);
	_filename = filename;
	if(!_file)
	{
		std::cout << RED << "couldn't open body file" << RESET << std::endl;
	}
	_file.seekg(0, std::ios::end);
	std::streampos fileSize = _file.tellg();
	_file.seekg(0, std::ios::beg);
	// should add something when the file fails to open due to permissions...
	_contentLength = fileSize;
	// _file.close();
}

void Response::generateDirectoryListing(std::string dirPath) {
	DIR* dir;
	struct dirent* entry;

	dir = opendir(dirPath.c_str());
	// gotta add something when the directory fails to open
	std::ofstream htmlFile(DIRECTORY_LISTING_FILENAME);
	// gotta add somethign when the file fails to be created

	//html Header
	htmlFile << "<!DOCTYPE html><html><head><title>directory listing</title></head><body><ul>";
	//read directory entries
	while((entry = readdir(dir)) != NULL) {
		std::string entryName = entry->d_name;
		//exclude current and parent directories
		if(entryName == "." || entryName == "..")
			continue ;
		struct stat entryStat;
		std::string entryPath  = dirPath + "/" + entryName;
		if(stat(entryPath.c_str(), &entryStat) != 0) {
			std::cerr << RED << "failed to get file info for : " << entryPath << std::endl;
			continue;
		}

		htmlFile << "<li><a href=\"" << _request.getRequestURI() << entryName << "\">" <<  entryName << "</a></li>";
	}
	htmlFile << "</ul></body></html>";
	closedir(dir);
	htmlFile.close();
}


void Response::sendResponseBody(std::string filename) {
	size_t chunkSize = RESPONSE_BUFFER_SIZE;
	if((_contentLength - _totalBytesSent) < chunkSize)
		chunkSize = _contentLength - _totalBytesSent;
	std::vector<char> buffer(chunkSize);
	if(!_file.is_open())
		_file.open(filename);
	if(!_file)
		std::cout << RED << "couldn't open file" << RESET << std::endl;
	_file.read(buffer.data(), chunkSize);
	size_t bytesSent = send(_writeSocket, buffer.data(), chunkSize, 0);
	if(bytesSent < 0) {
		_sendFailed = true;
		return;
		// gotta add something to resend the response from scratch when send fails
	}
	_totalBytesSent += bytesSent;
	
	std::cout << YELLOW << " total bytes sent : " << _totalBytesSent << std::endl;
	std::cout << "chunk size " << chunkSize << std::endl;
	std::cout << "filename " << filename << std::endl;
	std::cout << "content length : " << _contentLength << RESET << std::endl;
	if(_totalBytesSent  == _contentLength)
	{
		_file.close();
		std::cout << "am here" << std::endl;
		_isResponseSent = true;
		if(_request.getresourceType() == "directory"){
			std::remove(filename.c_str());
		}
	}
}


void Response::setStartLine() {
	_startLine = "HTTP/1.1 " + std::to_string(_statusCode) + _statusCodeMap[_statusCode] + "\r\n";
}

void Response::setHeaders() {
	_headers =  "Content-Type: text/html\r\n"
							"Content-Length: " + std::to_string(_contentLength) + "\r\n"
							"\r\n";
}

void Response::sendHeaders(std::string requestedResource) {
	setContentLength(requestedResource);
	setHeaders();
	std::string initialResponse = _startLine + _headers;
	size_t bytesSent = send(_writeSocket, initialResponse.c_str(), initialResponse.length(), 0);
	if(bytesSent < 0) {
		_sendFailed = true;
		return ;
	}
	_headersAreSent = true;
}

void Response::responseClass200() {
	if(_request.getMethod() == "GET") {
		if(_request.getresourceType() == "file")
		{
			if(_headersAreSent == false)
				sendHeaders(_request.getRequestedresource());
			else
				sendResponseBody(_request.getRequestedresource());
		}
		else {
			if(_headersAreSent == false) {
				generateDirectoryListing(_request.getRequestedresource());
				sendHeaders(DIRECTORY_LISTING_FILENAME);
			}
			else
			{
				sendResponseBody(DIRECTORY_LISTING_FILENAME);
			}
		}
	}
	/*
	if (post)
		upload the post request body
	if(delete) 
		delete success 
	*/
}

void Response::responseClass300(){

}

void Response::responseClass400(){}

void Response::responseClass500(){}

void Response::sendResponse() {
	setStartLine();
	if(_statusCode >= 200 && _statusCode < 300)
		responseClass200();
	else if(_statusCode >= 300 && _statusCode < 400)
		responseClass300();
	else if(_statusCode >= 400 && _statusCode < 500)
		responseClass400();
	else if(_statusCode >= 500)
		responseClass500();
}

	
void Response::initializeStatusCodeMap() {
	_statusCodeMap.insert(std::make_pair(301,  "Moved Permanently"));
	_statusCodeMap.insert(std::make_pair(400,  "Bad request"));
	_statusCodeMap.insert(std::make_pair(403,  "Forbidden"));
	_statusCodeMap.insert(std::make_pair(404,  "Not Found"));
	_statusCodeMap.insert(std::make_pair(405,  "Method not Allowed"));
	_statusCodeMap.insert(std::make_pair(409,  "Conflict"));
	_statusCodeMap.insert(std::make_pair(413,  "Request Entity Too Large"));
	_statusCodeMap.insert(std::make_pair(414,  "Request-URI Too Long"));
	_statusCodeMap.insert(std::make_pair(500,  "Internal Server Error"));
	_statusCodeMap.insert(std::make_pair(501,  "Not Implemented"));
	_statusCodeMap.insert(std::make_pair(200,  "OK"));
	_statusCodeMap.insert(std::make_pair(201,  "Created"));
	_statusCodeMap.insert(std::make_pair(204,  "No Content"));
}