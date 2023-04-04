#include"HttpMessage.cpp"

HttpMessage::HttpMessage() {}

HttpMessage::HttpMessage(HttpMessage const& other) {}

HttpMessage& HttpMessage::operator=(HttpMessage const& other) {}

HttpMessage::~HttpMessage() {}

const std::string& HttpMessage::getStartLine() const {return(_StartLine);}

const std::string& HttpMessage::getBody() const {return(_Body);}

const std::map<std::string, std::string>& getHeaders() const {return(_Headers);}

void HttpMessage::setStarLine(std::string StartLine) {_StartLine = StartLine; }

void HttpMessage::setBody(std::string Body) { _Body = Body; }

void HttpMessage::setHeaders(std::string name, std::string value) { _Headers[name] = value; }

void HttpMessage::parse(const std::string& Message) {
	std::vector<std::string> lines;

	int len;
	int pos = 0;
	while((end = Message.find("\r\n")) != Message.end()) {
		lines.push_back(Message.substr(pos , len - pos));
		pos = end + 2;
	}
	if(pos < Message.size())
		lines.push_back(Message.substr(pos));
	if(!lines.empty())
		setStartLine(lines[0]);

	std::vector<std::string>::iterator It;
	for(It = lines.begin() + 1; It != lines.end(); It++) {
		if(std::empty(*It)) {
			setBody(std::string(It++), lines.end());
			break ;
		}
		else {
			int colonPos;
			if((colonPos = It->find(":")) != std::string::npos) {
				std::string name = It->substr(0, colonPos);
				std::string value = It->substr(colonPos+1)
				setHeaders(name, value);
			}
		}

	}
}

/*
// pseudo code

  split messages  into lines with "\r\n"
  check if there's something left in the message after the last "\r\n" and save it tpp

  parse the start line

  parse the rest
  iterate through the lines and check if theres a line is empty which marks the end of the headers and start parsing the body
  else
  parse the header and add it to the headers map 
  
*/