#pragma once
#include "Http.hpp"
#include "HttpMessage.hpp"
#include "Request.hpp"

#define RESPONSE_BUFFER_SIZE 1024
#define DIRECTORY_LISTING_FILENAME "/tmp/directory_listing.html"
#define POST_201_BODY "created succefully !!!"
#define DELETE_204_BODY "No content"

class Request;

class Response : public HttpMessage {
	public :
		Response(Request &Request, int clientSock);
		Request& getRequest();
		void sendResponse();
		bool sendFailed();
		bool isResponseSent();
	private :
		void 		initializeStatusCodeMap();
		// methods needed for headers
		void 		setStartLine();
		void 		setHeaders(std::string contentLength);
		std::string	setMIMEtype(std::string filename);
		std::string	setFileContentLength(std::string filename);
		// send the header with the startLine
		void 		formatHeadersAndStartLine();
		// send the response
		void		responseSuccess();
		void 		responseError();
		// send reponse body (file/string body)
		void 		sendResponseFile(std::string filename);
		void 		sendResponseBody();
		void 		sendDefaultErrorPage();
		// html files generators
		void 		generateDirectoryListing(std::string dirPath);
		void 		generateErrorPage();

		// the request associated to each response instance
		Request&							_request;
		// response main attributes
		int 								_writeSocket;
		bool 								_isResponseSent;
		// mainly needed to get the server error_pages
		Server* 							_server;
		
		// values needed for the headers
		std::string 						_headerLocationValue;
		size_t 								_contentLength;
		int 								_statusCode;
		std::map<int, std::string> 			_statusCodeMap;
		std::map<std::string, std::string>	_headers;
		bool 								_headersAreSent;

		// errors handling related attributes
		bool 								_sendFailed;
		// the body related attributes
		size_t 								_totalBytesSent;
		std::ifstream 						_file;
		std::string 						_filename;
		std::map<int, std::string> 			_errorPages;
		std::string 						_body;
};



/*
Response status codes classes :
	1xx : informational
	2xx : success
	3xx : redirection
	4xx : client error
	5xx : server error
Response structure :
	status line = Http-version SP status-code SP reason-pharse CLRF
	entity headers : 
		-- "allow" header : lists the set of methods supported by the resource identified by the request URI, it MUST be present in a 405 (method not allowed response)
		-- Location : for 201 the location is that of the new resource, for 3xx responses, the location SHOULD indicate the server's preferred URI for automatic redirection to the resource.
		-- retry-after : this field MAY also be used with any 3xx response to indicate the minimum time the user-agent is asked to wait before issuing the redirected request (it can have either an HTTP-date or an integer number of seconds) after the time of the response
		-- content-length : indicates the length the size of the entity-body
		-- content type : indicates the media type of the entity-body sent to the recipient (in the case of a get method, i should check the file extension and then search in the mime types for the mim type of that extension)
*/


// to be added in the request parsing : not only you check if the request has "/" which will implicate that it's directory. it should also be checked of it's a directory or a file 
