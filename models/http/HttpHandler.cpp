#include "HttpHandler.hpp"

void HttpHandler::handleRequest(Client &client)
{
	std::string &rawRequestBytes = client.inBuff;
	HttpRequest &request = client.request;
	HttpResponse &response = client.response;
	try
	{
		HttpRequestParser::parseRawRequest(request, rawRequestBytes);
		// std::cout << std::endl;
		// std::cout << request << std::endl;
		if (request.getMethod().compare("GET") == 0)
			return HttpHandlerGET::handleGetRequest(request, response);
		if (request.getMethod().compare("POST") == 0)
			return HttpHandlerPOST::handlePostRequest(request, response);
		if (request.getMethod().compare("DELETE") == 0)
			return HttpHandlerDELETE::handleDeleteRequest(request, response);
	}
	catch (const Http400BadRequestException &e)
	{
		request.addHeader("Connection", "close");
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
	catch (const Http414UriTooLongException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_414_URI_TOO_LONG),
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
	catch (const Http505HttpVersionNotSupportedException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_505_HTTP_VERSION_NOT_SUPPORTED),
								e.what(),
								request.getHeaders(),
								request.getBody());
	}
}
