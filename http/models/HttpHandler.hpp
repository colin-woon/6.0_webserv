#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "HttpHandlerGET.hpp"

class HttpHandler : public HttpHandlerGET
{
public:
	static void handleRequest(HttpRequest &request, HttpResponse &response);
};

#endif
