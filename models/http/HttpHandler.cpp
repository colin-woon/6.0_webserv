#include "HttpHandler.hpp"

void HttpHandler::handleRequest(Client &client)
{
	std::string &rawRequestBytes = client.inBuff;
	HttpRequest &request = client.request;
	HttpResponse &response = client.response;
	try
	{
		HttpUtils::parseRawRequest(request, rawRequestBytes);
		// std::cout << std::endl;
		// std::cout << request << std::endl;
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
	catch (const Http405MethodNotAllowedException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_405_METHOD_NOT_ALLOWED),
								e.what(),
								request.getHeaders(),
								request.getBody());
	}
	catch (const Http501NotImplementedException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_501_NOT_IMPLEMENTED),
								e.what(),
								request.getHeaders(),
								request.getBody());
	}
}
