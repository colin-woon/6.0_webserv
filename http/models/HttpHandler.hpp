#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpStatusCodes.hpp"
#include <fstream>
#include <string>
#include <sstream>

class HttpHandler
{
public:
	HttpHandler();
	HttpHandler(const HttpHandler &other);
	HttpHandler &operator=(const HttpHandler &other);
	~HttpHandler();

	HttpResponse &handleRequest(HttpRequest &request);
	HttpResponse &handleGetRequest(HttpRequest &request);
};

#endif
