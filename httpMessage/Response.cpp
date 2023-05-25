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
	if(_request.getServerIndex() !=  -1)
	{
		_server = ServerFarm::getInstance()->getServers()[_request.getServerIndex()];
		_errorPages = _server->getErrorPage();
	}
}

Request& Response::getRequest() {return(_request);}

bool Response::sendFailed(){return(_sendFailed);}
bool Response::isResponseSent(){return(_isResponseSent);}


void Response::rebuildResponseErr(int statusCode) {
	_statusCode = statusCode;
	_headersAreSent = false;
	throw(Http::ResponseErrorException("hehe"));
}

void Response::generateDirectoryListing(std::string dirPath) {
	DIR* dir;
	struct dirent* entry;

	dir = opendir(dirPath.c_str());
	if(!dir)
		rebuildResponseErr(403);
	std::ofstream htmlFile(DIRECTORY_LISTING_FILENAME);
	if(!htmlFile)
		rebuildResponseErr(500);

	//html Header
	htmlFile << "<html><head><title>directory listing</title></head><body><ul>";
	//read directory entries
	while((entry = readdir(dir)) != NULL) {
		std::string entryName = entry->d_name;
		//exclude current and parent directories
		if(entryName == "." || entryName == "..")
			continue ;
		std::string entryPath  = dirPath + "/" + entryName;

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

void Response::sendResponseFile() {
	size_t chunkSize = RESPONSE_BUFFER_SIZE;
	if((_contentLength - _totalBytesSent) < chunkSize)
		chunkSize = _contentLength - _totalBytesSent;
	std::vector<char> buffer(chunkSize);
	if(!_file.is_open())
		_file.open(_filename, std::ios::in);
	if(!_file)
		rebuildResponseErr(403);
	_file.read(buffer.data(), chunkSize);
	size_t bytesSent = send(_writeSocket, buffer.data(), chunkSize, 0);
	if(bytesSent < 0) {
		rebuildResponseErr(500);
		_sendFailed = true;
		return;
	}
	_totalBytesSent += bytesSent;
	if(_totalBytesSent  == _contentLength)
	{
		_file.close();
		_isResponseSent = true;
		if(_request.getresourceType() == "directory"){
			std::remove(_filename.c_str());
		}
	}
}


void Response::setStartLine() {
	_startLine = "HTTP/1.1 " + std::to_string(_statusCode) + " " + _statusCodeMap[_statusCode] + "\r\n";
}

std::string Response::setMIMEtype(std::string filename) {
	std::string mimeType = "text/plain";
	size_t pos = filename.find_last_of('.');
	std::string extension = "";
	if(pos != std::string::npos && pos != (filename.size() - 1))
		extension = filename.substr(pos + 1);
	std::map<std::string, std::string> MIMEmap = ServerFarm::getInstance()->getMIMEtypes();
	if(MIMEmap.find(extension) != MIMEmap.end())
	{
		mimeType = MIMEmap[extension];
	}
	return(mimeType);
}

std::string Response::setFileContentLength(std::string filename) {
	_file.open(filename);
	_filename = filename;
	if(!_file)
		rebuildResponseErr(403);
	_file.seekg(0, std::ios::end);
	std::streampos fileSize = _file.tellg();
	_file.seekg(0, std::ios::beg);
	// should add something when the file fails to open due to permissions... (403 forbidden)
	_contentLength = fileSize;
	_file.close();
	return (std::to_string(fileSize));
}

std::string Response::formatAllowedMethodsVector() {
	std::string ret;
	std::vector<std::string> vect = _server->getLocations()[_request.getLocationIndex()]->getAllowedMethods();
	for(size_t i = 0; i < vect.size(); i++)
		ret += vect[i] + " ";
	return(ret);
}

void Response::setHeaders(std::string contentLength) {
	setStartLine();
	_headers.insert(std::make_pair("Content-Type: ", setMIMEtype(_filename)));
	_headers.insert(std::make_pair("Content-Length: ", contentLength));
	if (_statusCode == 201)
		_headerLocationValue = _request.getUploadFilename();
	if(_statusCode == 301)
	{
		if(_request.getRequestURI()[_request.getRequestURI().size() -1] != '/')
			_headerLocationValue = _request.getRequestURI() + "/";
		else
			_headerLocationValue = _request.getRequestURI();
	}
	if(_statusCode == 405)
	{
		_headers.insert(std::make_pair("Allow: ", formatAllowedMethodsVector()));
	}
	_headers.insert(std::make_pair("Location: ", _headerLocationValue));
}


void Response::formatHeadersAndStartLine() {
	std::string initialResponse = _startLine;
	std::map<std::string, std::string>::iterator It;
	for(It = _headers.begin(); It != _headers.end(); It++)
		initialResponse += It->first + It->second + "\r\n";
	initialResponse += "\r\n";
	size_t bytesSent = send(_writeSocket, initialResponse.c_str(), initialResponse.length(), 0);
	if(bytesSent < 0) {
		rebuildResponseErr(500);
		_sendFailed = true;
	}
	std::cout << RED << initialResponse << RESET << std::endl;
	_headersAreSent = true;
}

void Response::responseSuccess() {
	if(_request.getMethod() == "GET") {
		if(_request.getresourceType() == "file")
		{
			if(_headersAreSent == false) {
				setHeaders(setFileContentLength(_request.getRequestedresource()));
				formatHeadersAndStartLine();
			}
			else
				/*
					if cgi output_filename not empty
					send cgi response body
				*/
				sendResponseFile();
		}
		else {
			if(_headersAreSent == false) {
				generateDirectoryListing(_request.getRequestedresource());
				setHeaders(setFileContentLength(DIRECTORY_LISTING_FILENAME));
				formatHeadersAndStartLine();
			}
			else
				sendResponseFile();
		}
	}
	else if(_request.getMethod() == "POST") {
		if(_headersAreSent == false) {
			_body = POST_201_BODY;
			setHeaders(std::to_string(_body.size()));
			formatHeadersAndStartLine();
		}
		else
		{
			//if(cgi_output_filename not empty)
			// send cgi response body
			sendResponseBody();
		}
	}
	else if(_request.getMethod() == "DELETE") {
		if(_headersAreSent == false) {
			_body = DELETE_204_BODY;
			setHeaders(std::to_string(_body.size()));
			formatHeadersAndStartLine();
		}
		else
		{
			sendResponseBody();
		}
	}
}

void Response::sendResponseBody() {
	size_t bytesSent = send(_writeSocket, _body.c_str(), _contentLength, 0);
	if(bytesSent < 0) {
		rebuildResponseErr(500);
		_sendFailed = true;
		return;
	}
	if(bytesSent == _contentLength)
	{
		_isResponseSent = true;
	}
	else
	{
		rebuildResponseErr(500);
		_sendFailed = true;
	}
}

void Response::sendDefaultErrorPage() {
	if(_headersAreSent == false) {
		generateErrorPage();
		_filename = "file.html";
		setHeaders(std::to_string(_body.size()));
		formatHeadersAndStartLine();
	}
	else
		sendResponseBody();
}

void Response::responseError(){
	if(_request.getServerIndex() != -1 &&  _errorPages.find(_statusCode) != _errorPages.end()) {
		if(_headersAreSent == false) {
			setHeaders(setFileContentLength(_errorPages[_statusCode]));
			formatHeadersAndStartLine();
		}
		else
			sendResponseFile();
	}
	else
		sendDefaultErrorPage();
}


void Response::sendResponse() {
	try {
		if(_request.getServerIndex() != -1 &&_request.is_location_has_redirection() == true) {
			_headerLocationValue = _server->getLocations()[_request.getLocationIndex()]->getRedirect();
			setHeaders("0");
			formatHeadersAndStartLine();
			_isResponseSent = true;
		}
		else if(_statusCode >= 200 && _statusCode < 302)
			responseSuccess();
		else if(_statusCode >= 400 && _statusCode < 600)
			responseError();
	}
	catch(Http::ResponseErrorException& e) {
		responseError();
	}
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