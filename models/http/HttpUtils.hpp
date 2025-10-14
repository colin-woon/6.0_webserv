#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>
#include <set>
#include <iostream>
#include <sstream>
#include <cstdio>
#include "HttpExceptions.hpp"

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
	static void skipEmptyLines(std::istringstream &requestStream, std::string &line);
	static void checkForNUL(std::string &line);
	static void processCarriageReturn(std::string &line);
	static void getMethodTargetVersion(HttpRequest &request, std::string &line);
	static void parseTarget(HttpRequest &request);
	static void handleEndOfHeaders(HttpRequest &request, std::istringstream &requestStream);
	static void validateWhitespaceBeforeHeaderField(std::string &line);
	static void parseHeaderKeyValuePair(std::string &line, HttpRequest &request);
};

#endif
