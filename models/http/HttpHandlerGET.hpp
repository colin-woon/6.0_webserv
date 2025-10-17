#ifndef HTTPHANDLERGET_HPP
#define HTTPHANDLERGET_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpExceptions.hpp"
#include "FileHandler.hpp"
#include "Router.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <dirent.h>

class HttpHandlerGET
{
private:
	HttpHandlerGET();
	HttpHandlerGET(const HttpHandlerGET &other);
	HttpHandlerGET &operator=(const HttpHandlerGET &other);
	~HttpHandlerGET();

protected:
	static void handleGetRequest(HttpRequest &request, HttpResponse &response, Router &router, const Server &serverConfig);
};

#endif
