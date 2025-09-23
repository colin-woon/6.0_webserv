#include "HttpHandlerPOST.hpp"

HttpHandlerPOST::HttpHandlerPOST() {}

HttpHandlerPOST::HttpHandlerPOST(const HttpHandlerPOST &other)
{
	(void)other;
}

HttpHandlerPOST &HttpHandlerPOST::operator=(const HttpHandlerPOST &other)
{
	(void)other;
	return *this;
}

HttpHandlerPOST::~HttpHandlerPOST() {}

void HttpHandlerPOST::handlePostRequest(HttpRequest &request, HttpResponse &response)
{
	printf("Hello FROM POST\n");
}
