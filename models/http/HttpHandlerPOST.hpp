#ifndef HTTPHANDLERPOST_HPP
#define HTTPHANDLERPOST_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpExceptions.hpp"
#include "FileHandler.hpp"
#include "Router.hpp"
#include "Cookie.hpp"

class HttpHandlerPOST
{
private:
	HttpHandlerPOST();
	HttpHandlerPOST(const HttpHandlerPOST &other);
	HttpHandlerPOST &operator=(const HttpHandlerPOST &other);
	~HttpHandlerPOST();

protected:
	static void handlePostRequest(HttpRequest &request, HttpResponse &response, Router &router);
};

#endif
