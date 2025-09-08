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

	void handleRequest(HttpRequest &request, HttpResponse &response);
	void handleGetRequest(HttpRequest &request, HttpResponse &response);
};

#endif
