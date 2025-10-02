#ifndef HTTPHANDLERDELETE_HPP
#define HTTPHANDLERDELETE_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpExceptions.hpp"
#include "FileHandler.hpp"

class HttpHandlerDELETE
{
private:
	HttpHandlerDELETE();
	HttpHandlerDELETE(const HttpHandlerDELETE &other);
	HttpHandlerDELETE &operator=(const HttpHandlerDELETE &other);
	~HttpHandlerDELETE();

protected:
	static void handleDeleteRequest(HttpRequest &request, HttpResponse &response);
};

#endif
