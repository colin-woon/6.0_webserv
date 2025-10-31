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

static std::string getCurrentHttpDate()
{
	char buf[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return std::string(buf);
}

static std::string generateDefaultErrorPage(int statusCode, const std::string &statusMessage)
{
	std::stringstream ss;
	ss << "<!DOCTYPE html>\n"
	   << "<html lang=\"en\">\n"
	   << "<head>\n"
	   << "    <meta charset=\"UTF-8\">\n"
	   << "    <title>Error " << statusCode << "</title>\n"
	   << "    <style>\n"
	   << "        body { font-family: sans-serif; text-align: center; padding-top: 50px; }\n"
	   << "        h1 { font-size: 48px; }\n"
	   << "    </style>\n"
	   << "</head>\n"
	   << "<body>\n"
	   << "    <h1>Error " << statusCode << "</h1>\n"
	   << "    <p>" << statusMessage << "</p>\n"
	   << "    <hr>\n"
	   << "    <p><em>42_Webserv</em></p>\n"
	   << "</body>\n"
	   << "</html>";
	return ss.str();
}

static void getCustomErrorPage(const Server &serverConfig, const Location *locationConfig, const HttpException &e, HttpResponse &response)
{
	std::string relativePathToErrorPage;
	bool found = false;

	if (locationConfig != NULL)
	{
		std::map<int, std::string>::const_iterator it = locationConfig->error_pages.find(e.getStatusCodeDigit());
		if (it != locationConfig->error_pages.end())
		{
			relativePathToErrorPage = it->second;
			found = true;
		}
	}

	if (!found)
	{
		std::map<int, std::string>::const_iterator it = serverConfig.error_pages.find(e.getStatusCodeDigit());
		if (it != serverConfig.error_pages.end())
		{
			relativePathToErrorPage = it->second;
			found = true;
		}
	}

	if (!found)
		throw std::runtime_error("No custom error page configured.");

	std::string resolvedPathToErrorPage = serverConfig.root + relativePathToErrorPage;
	std::ifstream file(resolvedPathToErrorPage.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Custom error page not found.");
	else
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();
		file.close();

		response.setStatusCode(e.getStatusCodeString());
		response.addHeader("Content-Type", "text/html");
		response.addHeader("Server", "42_Webserv");
		response.addHeader("Date", getCurrentHttpDate());

		std::stringstream contentLength;
		contentLength << content.length();
		response.addHeader("Content-Length", contentLength.str());
		response.setBody(content);
	}
}

static void getDefaultErrorPage(HttpResponse &response, const HttpException &e)
{
	response.setStatusCode(e.getStatusCodeString());
	response.setBody(generateDefaultErrorPage(e.getStatusCodeDigit(), e.what()));
	response.addHeader("Content-Type", "text/html");
	response.addHeader("Server", "42_Webserv");
	response.addHeader("Date", getCurrentHttpDate());

	std::stringstream contentLength;
	contentLength << response.getBody().length();
	response.addHeader("Content-Length", contentLength.str());
}

static void handleHttpException(const Server &serverConfig, const Location *locationConfig, const HttpException &e, HttpResponse &response)
{
	try
	{
		getCustomErrorPage(serverConfig, locationConfig, e, response);
	}
	catch (const std::exception &fallback)
	{
		getDefaultErrorPage(response, e);
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
		if (statusCode == "400" || statusCode[0] == '5')
		{
			response.addHeader("Connection", "close");
			client.closeFlag = true;
		}
		else
			response.addHeader("Connection", "keep-alive");
	}
}
