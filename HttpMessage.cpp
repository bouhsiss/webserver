#include"HttpMessage.hpp"

HttpMessage::HttpMessage() {}

HttpMessage::HttpMessage(HttpMessage const& other) {}

HttpMessage& HttpMessage::operator=(HttpMessage const& other) {return(*this);}

HttpMessage::~HttpMessage() {}

const std::string& HttpMessage::getStartLine() const {return(_StartLine);}

const std::string& HttpMessage::getBody() const {return(_Body);}

const std::map<std::string, std::string>& HttpMessage::getHeaders() const {return(_Headers);}

void HttpMessage::setStartLine(std::string StartLine) {_StartLine = StartLine; }

void HttpMessage::setBody(std::string Body) { _Body = Body; }

void HttpMessage::setHeaders(std::string name, std::string value) { _Headers[name] = value; }

void HttpMessage::parse(const std::string& Message) {
	std::vector<std::string> lines;

	int len;
	int pos = 0;
	while(Message.find("\r\n", pos) != std::string::npos) {
		len = Message.find("\r\n", pos);
		lines.push_back(Message.substr(pos , len - pos));
		pos = len + 2;
	}
	if(pos < Message.size())
		lines.push_back(Message.substr(pos));
	if(!lines.empty())
		setStartLine(lines[0]);

	std::vector<std::string>::iterator It;
	for(It = lines.begin() + 1; It != lines.end(); It++) {
		if(It->empty()) {
			std::string body;
			for( ; It != lines.end(); It++)
				body.append(*It).append("\r\n");
			setBody(body);
			break ;
		}
		else {
			int colonPos;
			if((colonPos = It->find(":")) != std::string::npos) {
				std::string name = It->substr(0, colonPos);
				std::string value = It->substr(colonPos+1);
				setHeaders(name, value);
			}
		}

	}
}
