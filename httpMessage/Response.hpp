#pragma once
#include "Http.hpp"
#include "HttpMessage.hpp"
#include "Request.hpp"

class Response : public HttpMessage {
	public :
		Response(Request &Request);
	private :
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
	
*/