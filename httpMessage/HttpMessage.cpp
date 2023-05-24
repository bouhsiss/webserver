#include"HttpMessage.hpp"

HttpMessage::HttpMessage(): _body_length(0),_sl_complete(false),_hd_complete(false),_b_complete(false){
	_filename = "/Users/hassan/Desktop/request2.0/tmp/"+random_filename()+"._Body";
	_chunk_size = -1;
	_bytes_read=0;
	_body_length=0;
}

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
std::string HttpMessage::random_filename() {
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
	//set start_line
	if (_sl_complete==false && _message.find("\r\n")!=std::string::npos )
	{
		_startLine = _message.substr(0,_message.find("\r\n"));
		//remove start_line from buffer
		_sl_complete = true;
		_message = _message.substr(_message.find("\r\n")+2);
	}
	//put all headers in one string
	if (_sl_complete==true&& _hd_complete ==false&& _message.find("\r\n\r\n")!=std::string::npos)
	{
		_hd_complete=true;
		if (_startLine.find("GET")!=std::string::npos || _startLine.find("DELETE")!=std::string::npos)
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
		}
	}
	//set body	
	if(_sl_complete == true && _hd_complete == true && _b_complete == false)
	{
		if (_message.empty())
		{
			_b_complete=true;
			return ;
		}
		_Body.open(_filename.c_str(),std::ios::out|std::ios::app);
		if (_Body.is_open())
		{
			if (_Headers.find("Transfer-Encoding")!=_Headers.end())//chunked body
			{
					if (_chunk_size!=-1 && _bytes_read != 1024)//last chunk
					{
						while (_message.length())
						{
							if (_chunk_size!=0)
								append_chunk();
							// else
							read_new_chunk();
						}
					}
					else
					{
						read_first_chunk();
						while ((int)_message.length() >=_chunk_size+10)
						{
							if (_chunk_size!=0)
								append_chunk();
							// else
							read_new_chunk();
						}
					}
			}
			else //normal body
			{
				//debug
				std::cerr<<"content-length == [ "<<_Headers["Content-Length"]<<"]"<<std::endl;
				std::cerr<<"body_length == ["<<_body_length<<"]"<<std::endl;
				std::cerr<<"message_length == ["<<_message.length()<<"]"<<std::endl;
				std::cerr<<"message == ["<<_message<<"]"<<std::endl;

				//end debug
				_Body<<_message;
				_body_length +=  _message.length();
				_message.clear();
				if ((_Headers.find("Content-Length")!=_Headers.end()&& (int)_body_length >= atoi(_Headers.find("Content-Length")->second.c_str())))
					_b_complete = true;
			}
			_Body.close();
		}
	}
}



//hepler functions
void 	HttpMessage::read_first_chunk(){
	if (_chunk_size == -1)//first chunk
	{
		//read chunk
		_chunk_size = std::stoi(_message.substr(0,_message.find("\r\n")),0,16);
		if (_chunk_size == 0)
		{
			_b_complete=true;
			_message.clear();
		}
		else
			_message = _message.substr(_message.find("\r\n")+2);//skip chunk_size
		//debugl
		// std::cerr<<"-------------read_first_chunk------------"<<std::endl;
		// std::cerr<<"_chunk_size1 == ["<<_chunk_size<<"]"<<std::endl;
		// std::cerr<<"_chunk1==["<<_message<<"]"<<std::endl;
		//end debug
		if (_chunk_size == (int) _message.length())//last chunk
		{

		}
	}
}

void	HttpMessage::read_new_chunk(){
	if (!_message.empty())
	{
		std::string line = _message.substr(0,_message.find("\r\n"));
		_chunk_size = std::stoi(line,0,16);
		//debug
		// std::cerr<<"-------------read_new_chunk-----------"<<std::endl;
		// std::cerr<<"_chunk_size2 == ["<<_chunk_size<<"]"<<std::endl;
		// std::cerr<<"_chunk2==["<<_message<<"]"<<std::endl;
		//end debug
		if (_chunk_size ==0)
		{
			_b_complete = true;
			_message.clear();
		}
		else
			_message = _message.substr(_message.find("\r\n")+2);
	}
}

void	HttpMessage::append_chunk(){
	std::string chunk_of_chunk;
	chunk_of_chunk = _message.substr(0,_chunk_size);
	_Body<<chunk_of_chunk;
	_message = _message.substr(_chunk_size+2);
	//debug
	// std::cerr<<"-------------append_chunk-----------"<<std::endl;
	// std::cerr<<"_chunk_size3 == ["<<_chunk_size<<"]"<<std::endl;
	// std::cerr<<"_chunk3==["<<chunk_of_chunk<<"]"<<std::endl;
	//end debug
	_chunk_size=0;
}
//old body
// while (_message.length())
// 					{
// 						if (_chunk_size!=0 && (int)_message.length() <=_chunk_size )//msg contains one chunk
// 						{
// 							if (_chunk_size == (int)_message.length())
// 								_flag = 2;
// 							_Body<<_message;
// 							_chunk_size-=_message.length();
// 							_message.clear();
// 						}
// 						else if(_chunk_size != 0 && !_message.empty()) //msg contain multiple chunks
// 						{
// 							if ((int)_message.length() == _chunk_size +2 || (int)_message.length() == _chunk_size +1)
// 							{
// 								if ((int)_message.length() == _chunk_size +2)
// 									_flag =0;
// 								else
// 									_flag = 1;
// 								_chunk_size=0;
// 								_Body<<_message;
// 								_message.clear();
// 							}
// 							else
// 							{
// 								std::string chunk_of_chunk;
// 								chunk_of_chunk = _message.substr(_flag,_chunk_size);
// 								_Body<<chunk_of_chunk;
// 								_message = _message.substr(_chunk_size+2+_flag);
// 								_chunk_size=0;
// 								_flag =0;
// 							}
// 						}
// 						if (_chunk_size==0 && !_message.empty())
// 						{
// 							std::string line;
// 							// line  = _message.substr(0,_message.find("\r\n"));//skip CRLF
// 							line = _message.substr(_flag,_message.find("\r\n"));//get  chunk_size
							
// 							//debug
// 							std::cerr<<"_chunk == ["<<_message<<"]"<<std::endl;
// 							//end debug
// 							_message = _message.substr(_message.find("\r\n",_flag)+2);
// 							_flag=0;
// 							//debugl
// 							std::cerr<<"_chunk_size3 == ["<<line<<"]"<<std::endl;
// 							//end debug
// 							_chunk_size = std::stoi(line,0,16);
// 							if (_chunk_size ==0)
// 							{
// 								_b_complete = true;
// 								break;
// 							}
// 						}
