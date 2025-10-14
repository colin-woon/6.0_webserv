#ifndef HTTPREQUESTPARSER_HPP
#define HTTPREQUESTPARSER_HPP

#include "HttpUtils.hpp"
#include "../parsing/Server.hpp"

class HttpRequestParser
{
public:
	static void parseRawRequest(HttpRequest &request, const std::string &rawRequestBytes, const Server &serverConfig);
};

#endif
