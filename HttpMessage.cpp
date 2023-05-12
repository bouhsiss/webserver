#include"HttpMessage.hpp"

HttpMessage::HttpMessage() {}

HttpMessage::HttpMessage(HttpMessage const& other) {
	(void)other;
}

HttpMessage& HttpMessage::operator=(HttpMessage const& other) {
	(void)other;
	return(*this);
	}

HttpMessage::~HttpMessage() {}


void HttpMessage::setHeaders(std::string name, std::string value) { _Headers[name] = value; }

HttpMessage::HttpMessage(std::string& Message):_filename(tmpnam(NULL)),_body_length(0){
	//skip CRLF
	//added code
	while(Message.find("\r\n",0) == 0)
		Message = Message.substr(2);

	//set start_line
	if (Message.find("\r\n")!=std::string::npos){
		_StartLine = Message.substr(0,Message.find("\r\n"));
		//remove start_line from body
		Message = Message.substr(Message.find("\r\n")+2);
	}
	//put all headers in one string
	std::string heads;
	if (Message.find("\r\n\r\n")!=std::string::npos)
	{
		heads= Message.substr(0,Message.find("\r\n\r\n")+2);
		//remove headers from  message
		Message = Message.substr(Message.find("\r\n\r\n")+4);
	}
	//set body	
	
	_Body.open(_filename,std::ios::in);
	if (_Body.is_open())
	{
		_Body<<Message;
		_body_length +=  Message.length();
		_Body.close();
	}
	//add headers in map
	std::string name,value;

	while (heads.length() > 0)
	{
		name = heads.substr(0,heads.find(":"));
		heads = heads.substr(heads.find(":")+1);
		value = heads.substr(0,heads.find("\r\n"));
		heads = heads.substr(heads.find("\r\n")+2);
		setHeaders(name,value);
		// if the header occupies more than one line append the rest of the header value  
		while (heads[0] && (heads[0]==' ' || heads[0]==9))//SP or HT
		{
			value = heads.substr(0,heads.find("\r\n"));
			_Headers[name].append(value);
			heads = heads.substr(heads.find("\r\n")+2);
		}
	}
}

void HttpMessage::append_chunk(std::string chunk){
	_Body.open(_filename,std::ios::in);
	if (_Body.is_open())
	{
		_Body<<chunk;
		_body_length +=  chunk.length();
		_Body.close();
	}
}
