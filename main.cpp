#include"HttpMessage.hpp"

int main() {
	std::string HttpRequest = "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: Mozilla/5.0\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!";

	HttpMessage base;
	base.parse(HttpRequest);

	std::cout << "start line : " << base.getStartLine() << std::endl;
	std::cout << "headers : " << std::endl;
	const std::map<std::string, std::string> ref = base.getHeaders();
	std::map<std::string, std::string>::const_iterator It;
	for(It = ref.begin(); It != ref.end(); It++) {
		std::cout << "key : " << It->first << " - value : " << It->second << std::endl;
	}
	std::cout << "body : " << base.getBody() << std::endl;
}