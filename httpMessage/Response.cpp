#include "Response.hpp"

Response::Response(Request &request, int writeSock): _request(request), _writeSocket(writeSock) {}

Request& Response::getRequest() {return(_request);}

void Response::responseClass200(){}

void Response::responseClass300(){}

void Response::responseClass400(){}

void Response::responseClass500(){}

void Response::sendResponse() {
	(void)_writeSocket;
	_statusCode = _request.getStatusCode();
	if(_request.getStatusCode() >= 200)
		responseClass200();
	else if(_request.getStatusCode())
		std::cout << "hehe" << std::endl;
}