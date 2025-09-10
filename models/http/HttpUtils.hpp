#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>

class HttpRequest;
class HttpHandler;
class HttpExceptions;

class HttpUtils
{
private:
	HttpUtils();
	HttpUtils(const HttpUtils &other);
	HttpUtils &operator=(const HttpUtils &other);
	~HttpUtils();

public:
	static void parseRawRequest(HttpRequest &request, const std::string &rawRequestBytes);
};

#endif
