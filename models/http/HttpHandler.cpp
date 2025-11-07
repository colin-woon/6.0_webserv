#include "HttpHandler.hpp"

static void checkAllowedMethods(Router &router, HttpRequest &request)
{
	if (router.locationConfig)
	{
		if (!router.locationConfig->allowedMethods.empty())
		{
			bool methodFound = false;
			for (size_t i = 0; i < router.locationConfig->allowedMethods.size(); i++)
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

static void handleRedirection(HttpResponse &response, Router &router)
{
	response.setStatusCode(HttpException::statusCodeToString(router.locationConfig->redirect.first));
	response.addHeader("Location", router.locationConfig->redirect.second);
	response.addHeader("Content-Length", "0");
	response.addHeader("Connection", "keep-alive");
	return;
}

void HttpHandler::handleRequest(Client &client)
{
	std::string &rawRequestBytes = client.inBuff;
	HttpRequest &request = client.request;
	HttpResponse &response = client.response;
	const Server &serverConfig = *client.serverConfig;
	const std::string &requestMethod = request.getMethod();

	Router router;

	try
	{
		HttpRequestParser::parseRawRequest(request, rawRequestBytes, serverConfig);
		router.getLocationConfig(request, serverConfig);
		if (router.locationConfig && !router.locationConfig->redirect.second.empty())
			return handleRedirection(response, router);
		checkAllowedMethods(router, request);
		router.getResolvedPath(request, serverConfig);
		if (router.locationConfig && !router.locationConfig->cgi.empty())
		{
			CGI cgi(client, serverConfig);
			if (client.srvLoop_)
				return cgi.handleCGI(client, router);
		}
		else if (request.getPath() == "/api/uploads" && requestMethod == "GET")
			return HttpHandlerGET::handleGetRequestAllFiles(request, response, router);
		else if (request.getPath().find("/api/download") == 0 && requestMethod == "GET")
			return HttpHandlerGET::handleGetRequestDownload(request, response, router);
		else if (router.locationConfig && !router.locationConfig->upload_store.empty())
		{
			if (requestMethod == "POST")
				return HttpHandlerPOST::handlePostRequest(request, response, router);
			if (requestMethod == "DELETE")
				return HttpHandlerDELETE::handleDeleteRequest(request, response, router);
		}
		if (requestMethod == "GET")
			return HttpHandlerGET::handleGetRequest(request, response, router, serverConfig);
	}
	catch (const HttpException &e)
	{
		handleHttpException(serverConfig, router.locationConfig, e, response);
		const std::string &statusCode = response.getStatusCode();
		std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Connection");
		if (it != request.getHeaders().end() && it->second == "close")
		{
			response.addHeader("Connection", "close");
			client.closeFlag = true;
			return ;
		}
		if (statusCode == "400" || statusCode[0] == '5')
		{
			response.addHeader("Connection", "close");
			client.closeFlag = true;
		}
		else
			response.addHeader("Connection", "keep-alive");
	}
}
