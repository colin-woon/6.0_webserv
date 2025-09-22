#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>
#include <set>

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
	static const size_t MAX_URI_LENGTH = 8000;
	static void parseRawRequest(HttpRequest &request, const std::string &rawRequestBytes);
};

#endif
