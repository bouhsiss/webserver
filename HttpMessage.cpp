#include"HttpMessage.hpp"

HttpMessage::HttpMessage(): _body_length(0),_sl_complete(false),_hd_complete(false),_b_complete(false){}

HttpMessage::HttpMessage(HttpMessage const& other) {
	(void)other;
}

HttpMessage& HttpMessage::operator=(HttpMessage const& other) {
	(void)other;
	return(*this);
	}

HttpMessage::~HttpMessage() {}


void HttpMessage::setHeaders(std::string name, std::string value) { _Headers[name] = value; }

//generate random filename
std::string random_filename() {
	int length = 6;
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int charset_size = sizeof(charset) - 1;
    std::string result(length, '\0');
    std::srand(std::time(nullptr));
    for (int i = 0; i < length; ++i) {
        result[i] = charset[std::rand() % charset_size];
    }
    return result;
}


void HttpMessage::parse(){
	//skip CRLF
	//added code
	
	_filename = random_filename();

	//set start_line
	if (_sl_complete==false && _message.find("\r\n")!=std::string::npos )
	{
		_StartLine = _message.substr(0,_message.find("\r\n"));
		//remove start_line from buffer
		_sl_complete = true;
		_message = _message.substr(_message.find("\r\n")+2);
	}
	//put all headers in one string
	if (_hd_complete ==false&& _message.find("\r\n\r\n")!=std::string::npos)
	{
		_hd_complete=true;
		if (_StartLine.find("GET")!=std::string::npos || _StartLine.find("DELETE")!=std::string::npos)
			_b_complete=true;
		std::string heads;
		heads= _message.substr(0,_message.find("\r\n\r\n")+2);
		//remove headers from  message
		_message = _message.substr(_message.find("\r\n\r\n")+4);
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
	//set body	
	if(_sl_complete == true && _hd_complete == true && _b_complete != true)
	{
		_Body.open(_filename.c_str(),std::ios::in);
		if (_Body.is_open())
		{
			_Body<<_message;
			if ((_Headers.find("Transfer-Encoding")!=_Headers.end() && _message.find("0/r/n"))||
				(_Headers.find("Content-Length")!=_Headers.end()&& (int)_body_length >= atoi(_Headers.find("Content-Length")->second.c_str())))
				{
					_b_complete = true;
				}

			_message = "";
			_body_length +=  _message.length();
			_Body.close();
		}
	}
}
