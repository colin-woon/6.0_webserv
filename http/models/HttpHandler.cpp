#include "HttpHandler.hpp"

void HttpHandler::handleRequest(HttpRequest &request, HttpResponse &response, const std::string &rawRequestBytes)
{
	try
	{
		HttpUtils::parseRawRequest(request, rawRequestBytes);
		std::cout << std::endl;
		std::cout << request << std::endl;
		if (request.getMethod().compare("GET") == 0)
			return HttpHandlerGET::handleGetRequest(request, response);
	}
	catch (const Http400BadRequestException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_400_BAD_REQUEST),
								e.what(),
								request.getHeaders(),
								request.getBody());
	}
	catch (const Http404NotFoundException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_404_NOT_FOUND),
								e.what(),
								request.getHeaders(),
								request.getBody());
	}
}
