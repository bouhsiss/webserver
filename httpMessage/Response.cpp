#include "Response.hpp"

Response::Response(Request &request, int writeSock): _request(request), _writeSocket(writeSock) {
	initializeStatusCodeMap();
	_totalBytesSent = 0;
	_isResponseSent = -1;
	_contentLength = 0;
	_isResponseSent = false;
}

Request& Response::getRequest() {return(_request);}

void Response::setContentLength(std::string filename) {
	std::ifstream _file(filname);
	_file.seekg(0, std::ios::end);
	std::ostream fileSize = _file.tellg();
	_file.seekg(0, std::ios::beg);
	// should add something when the file fails to open due to permissions...
	_contentLength = fileSize;
}

std::string Response::generateDirectoryListing(std::string dirPath) {
	DIR* dir;
	struct dirent* entry;

	dir = opendir(dirPath.c_str());
	// gotta add something when the directory fails to open
	std::ofstream htmlFile("directory_listing.html");
	// gotta add somethign when the file fails to be created

	//html Header
	htmlFile << "<html><body><ul>";
	//read directory entries
	while((entry = readdir(dir)) != NULL) {
		std::string entryName = entry->d_name;
		//exclude current and parent directories
		if(entryName == "." || entryName == "..")
			continue ;
		struct stat entryStat;
		std::string entryPath  = dirPath + "/" + entry;
		if(stat(entryPtah.c_str(), &entryStat) != 0) {
			std::cerr << RED << "failed to get file info for : " << entryPath << std::endl;
			continue;
		}

		htmlFile << "<li><a href=\"" << filePath << "\">" <<  entryName << "</a></li>";
	}
	htmlFile << "</ul></body></html>";
	closedir(dir);
	htmlFile.close();
	return("directory_listing.html");
}


void Response::sendResponseBody(std::string filename) {
	char buffer[RESPONSE_BUFFER_SIZE];
	file.read(buffer, RESPONSE_BUFFER_SIZE);
	size_t bytesSent = send(_writeSocket, buffer, file.gcount());
	if(byteSent < 0) {
		_totalBytesSent = -1;
		return;
		// gotta add something to resend the response from scratch when send fails
	}
	_totalBytesSent += bytesSent;
	if(_totalBytesSent  == _contentLength)
	{
		_isResponseSent = true;
		if(_request.getresourceType() == "directory"){
			std::remove(filename)
		}
	}
}


void Response::setstartLine() {
	_startLine = "HTTP/1.1 " + std::to_string(_statusCode) + _statusCodeMap[_statusCode];
}

void Response::setHeaders() {
	std::string _headers =  "Content-Type: text/html\r\n"
							"Content-Length: " + std::to_string(_contentLength) + "\r\n"
							"\r\n"
}

void sendHeaders(std::string requestedResource) {
	setContentLength(requestedResource);
	setHeaders();
	std::string initialResponse = _startLine + _headers;
}

void Response::responseClass200() {
	if(_request.getMethod() == "GET") {
		if(_request.getresourceType() == "file")
		{
			if(_headersAreSent == false)
				sendHeaders(_request.getRequestedrsource());
			else
				sendResponseFile(_request.getRequestedresource());
		}
		else {
			if(_headerAreSent == false) {
				generateDirectoryListing(_request.getRequestedresource());
				sendHeaders("/tmp/directory_listing.html");
			}
			else
				sendResponseFile("/tmp/directory_listing.html");
		}
	}
	/*
	check method
	if(get)
		check if requested resource is file
			if yes return requested file
			if no generate autoindex
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
	_statusCode = _request.getStatusCode();
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