#include "HttpMessage.hpp"
#include "Request.hpp"
#include "Configuration.hpp"

namespace Http {
	void tokenize(std::string const &str, const char* delim, std::vector<std::string>& out);
	void trimSpaces(std::string &line);
}