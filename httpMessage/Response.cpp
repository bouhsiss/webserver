#include "Response.hpp"

Response::Response(Request &request, int writeSock): _request(request), _writeSocket(writeSock) {
	initializeStatusCodeMap();
	_totalBytesSent = 0;
	_isResponseSent = -1;
	_contentLength = 0;
	_isResponseSent = false;
	_headersAreSent = false;
	_sendFailed = false;
	_body = "";
	_statusCode = _request.getStatusCode();
	_servers = ServerFarm::getInstance()->getServers();
	_errorPages = _servers[_request.getServerIndex()]->getErrorPage();
}

Request& Response::getRequest() {return(_request);}

bool Response::sendFailed(){return(_sendFailed);}
bool Response::isResponseSent(){return(_isResponseSent);}

std::string Response::setContentLength(std::string filename) {
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
	return (std::to_string(fileSize));
}

void Response::generateDirectoryListing(std::string dirPath) {
	DIR* dir;
	struct dirent* entry;

	dir = opendir(dirPath.c_str());
	// gotta add something when the directory fails to open
	std::ofstream htmlFile(DIRECTORY_LISTING_FILENAME);
	// gotta add somethign when the file fails to be created

	//html Header
	htmlFile << "<html><head><title>directory listing</title></head><body><ul>";
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

void Response::generateErrorPage() {
	std::string errorPage = "<html><head><title>Error - "
							 + std::to_string(_statusCode) + "</title></head><body><h1> Error - " + std::to_string(_statusCode) + "</h1><p>" + _statusCodeMap[_statusCode] + "</p></body></html>";
	_contentLength = errorPage.size();
	_body = errorPage;
}

void Response::sendResponseFile(std::string filename) {
	size_t chunkSize = RESPONSE_BUFFER_SIZE;
	if((_contentLength - _totalBytesSent) < chunkSize)
		chunkSize = _contentLength - _totalBytesSent;
	std::vector<char> buffer(chunkSize);
	if(!_file.is_open())
		_file.open(filename, std::ios::in);
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
	if(_totalBytesSent  == _contentLength)
	{
		_file.close();
		_isResponseSent = true;
		if(_request.getresourceType() == "directory"){
			std::remove(filename.c_str());
		}
	}
}


void Response::setStartLine() {
	_startLine = "HTTP/1.1 " + std::to_string(_statusCode) + " " + _statusCodeMap[_statusCode] + "\r\n";
}

void Response::setHeaders(std::string contentLength) {
	_headerss.insert(std::make_pair("Content-Type: ", "text/html"));
	_headerss.insert(std::make_pair("Content-Length: ", contentLength));
	if( _statusCode == 301 && _request.getRequestURI()[_request.getRequestURI().size() -1] != '/')
		_headerLocationValue = _request.getRequestURI() + "/";
	// _headerss.insert(std::make_pair("Location: ", _headerLocationValue));
}


void Response::formatHeadersAndStartLine() {
	std::string initialResponse = _startLine;
	std::map<std::string, std::string>::iterator It;
	for(It = _headerss.begin(); It != _headerss.end(); It++)
		initialResponse += It->first + It->second + "\r\n";
	initialResponse += "\r\n";
	size_t bytesSent = send(_writeSocket, initialResponse.c_str(), initialResponse.length(), 0);
	if(bytesSent < 0) {
		_sendFailed = true;
		return;
	}
	std::cout << RED << initialResponse  << RESET << std::endl;
	_headersAreSent = true;
}

void Response::responseSuccess() {
	if(_request.getMethod() == "GET") {
		if(_request.getresourceType() == "file")
		{
			if(_headersAreSent == false) {
				setHeaders(setContentLength(_request.getRequestedresource()));
				formatHeadersAndStartLine();
			}
			else
				/*
					if cgi output_filename not empty
					send cgi response body
				*/
				sendResponseFile(_request.getRequestedresource());
		}
		else {
			if(_headersAreSent == false) {
				generateDirectoryListing(_request.getRequestedresource());
				setHeaders(setContentLength(DIRECTORY_LISTING_FILENAME));
				formatHeadersAndStartLine();
			}
			else
			{
				sendResponseFile(DIRECTORY_LISTING_FILENAME);
			}
		}
	}
	else if(_request.getMethod() == "POST") {
		if(_headersAreSent == false) {
			setHeaders(std::to_string(sizeof(POST_201_BODY)));
			formatHeadersAndStartLine();
		}
		else
		{
			//if(cgi_output_filename not empty)
			// send cgi response body
			send(_writeSocket, POST_201_BODY, sizeof(POST_201_BODY), 0);
			_isResponseSent = true;
		}
	}
	else if(_request.getMethod() == "DELETE") {
		if(_headersAreSent == false) {
			_headerss.insert(std::make_pair("Content-Type: ", "text/plain"));
			_headerss.insert(std::make_pair("Content-Length: ", std::to_string(sizeof(DELETE_204_BODY))));
			formatHeadersAndStartLine();
		}
		else
		{
			send(_writeSocket, DELETE_204_BODY, sizeof(DELETE_204_BODY), 0);
			_isResponseSent = true;
		}
	}
}

void Response::sendResponseBody() {
	size_t bytesSent = send(_writeSocket, _body.c_str(), _contentLength, 0);
	if(bytesSent < 0) {
		_sendFailed = true;
		return;
	}
	std::cout << "byteSent " << bytesSent << "content length " << _contentLength << std::endl;
	if(bytesSent == _contentLength)
	{
		_isResponseSent = true;
	}
	else
		_sendFailed = true;
}

void Response::sendDefaultErrorPage() {
	if(_headersAreSent == false) {
		generateErrorPage();
		setHeaders(std::to_string(_body.size()));
		formatHeadersAndStartLine();
	}
	else
	{
		sendResponseBody();
	}
}

void Response::responseError(){
	if(_errorPages.find(_statusCode) != _errorPages.end()) {
		if(_headersAreSent == false) {
			setHeaders(setContentLength(_errorPages[_statusCode]));
			formatHeadersAndStartLine();
		}
		else
			sendResponseFile(_errorPages[_statusCode]);
	}
	else
		sendDefaultErrorPage();
}


void Response::sendResponse() {
	setStartLine();
	if(_request.is_location_has_redirection() == true) {
		_headerLocationValue = _request.getRequestedresource();
		// might change the above line with getting the redirection value directly from the location
		setHeaders("0");
		formatHeadersAndStartLine();
		_isResponseSent = true;
	}
	else if(_statusCode >= 200 && _statusCode < 302)
		responseSuccess();
	else if(_statusCode >= 400 && _statusCode < 600)
		responseError();
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