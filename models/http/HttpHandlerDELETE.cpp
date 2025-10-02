#include "HttpHandlerDELETE.hpp"

HttpHandlerDELETE::HttpHandlerDELETE() {}

HttpHandlerDELETE::HttpHandlerDELETE(const HttpHandlerDELETE &other)
{
	(void)other;
}

HttpHandlerDELETE &HttpHandlerDELETE::operator=(const HttpHandlerDELETE &other)
{
	(void)other;
	return *this;
}

HttpHandlerDELETE::~HttpHandlerDELETE() {}

void HttpHandlerDELETE::handleDeleteRequest(HttpRequest &request, HttpResponse &response)
{
}
