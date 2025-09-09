#ifndef HTTPHANDLERGET_HPP
#define HTTPHANDLERGET_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpStatusCodes.hpp"
#include <fstream>
#include <string>
#include <sstream>

class HttpHandlerGET
{
private:
	HttpHandlerGET();
	HttpHandlerGET(const HttpHandlerGET &other);
	HttpHandlerGET &operator=(const HttpHandlerGET &other);
	~HttpHandlerGET();

protected:
	static void handleGetRequest(HttpRequest &request, HttpResponse &response);
};

#endif
