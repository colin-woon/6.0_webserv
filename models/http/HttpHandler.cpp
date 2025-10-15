#include "HttpHandler.hpp"

static void checkAllowedMethods(Router &router, HttpRequest &request)
{
	if (router.locationConfig)
	{
		if (!router.locationConfig->allowedMethods.empty())
		{
			bool methodFound = false;
			for (int i = 0; i < router.locationConfig->allowedMethods.size(); i++)
			{
				if (router.locationConfig->allowedMethods[i].compare(request.getMethod()) == 0)
				{
					methodFound = true;
					break;
				}
			}
			if (!methodFound)
				throw Http405MethodNotAllowedException();
		}
	}
}

void HttpHandler::handleRequest(Client &client)
{
	std::string &rawRequestBytes = client.inBuff;
	HttpRequest &request = client.request;
	HttpResponse &response = client.response;
	const Server &serverConfig = *client.serverConfig;
	Router router;

	try
	{
		HttpRequestParser::parseRawRequest(request, rawRequestBytes, serverConfig);
		router.getLocationConfig(request, serverConfig);
		router.getResolvedPath(request, serverConfig);
		checkAllowedMethods(router, request);
		// std::cout << std::endl;
		// std::cout << request << std::endl;
		if (router.locationConfig->path.compare("/cgi-bin/") == 0)
			return CGI::handleCGI(request, response, serverConfig, router);
		if (request.getMethod().compare("GET") == 0)
			return HttpHandlerGET::handleGetRequest(request, response, router);
		if (request.getMethod().compare("POST") == 0)
			return HttpHandlerPOST::handlePostRequest(request, response, router);
		if (request.getMethod().compare("DELETE") == 0)
			return HttpHandlerDELETE::handleDeleteRequest(request, response, router);
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
	catch (const Http413PayloadTooLargeException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_413_PAYLOAD_TOO_LARGE),
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
	catch (const Http502BadGatewayException &e)
	{
		response.createResponse(e.statusCodeToString(HTTP_502_BAD_GATEWAY),
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
