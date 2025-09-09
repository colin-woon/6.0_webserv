#include "HttpHandler.hpp"

void HttpHandler::handleRequest(HttpRequest &request, HttpResponse &response)
{
	if (request.getMethod().compare("GET") == 0)
		return HttpHandlerGET::handleGetRequest(request, response);
}
