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

static void getCustomErrorPage(const Server &serverConfig, const HttpException &e, HttpResponse &response)
{
	std::string relativePathToErrorPage = serverConfig.error_pages.at(e.getStatusCodeDigit());
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

static void handleHttpException(const Server &serverConfig, const HttpException &e, HttpResponse &response)
{
	try
	{
		getCustomErrorPage(serverConfig, e, response);
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
	Router router;

	try
	{
		HttpRequestParser::parseRawRequest(request, rawRequestBytes, serverConfig);
		router.getLocationConfig(request, serverConfig);
		if (router.locationConfig && !router.locationConfig->redirect.second.empty())
			return handleRedirection(response, router);
		router.getResolvedPath(request, serverConfig);
		checkAllowedMethods(router, request);
		if (router.locationConfig->path.compare("/cgi-bin/") == 0)
			return CGI::handleCGI(request, response, serverConfig, router);
		if (request.getMethod().compare("GET") == 0)
			return HttpHandlerGET::handleGetRequest(request, response, router, serverConfig);
		if (request.getMethod().compare("POST") == 0)
			return HttpHandlerPOST::handlePostRequest(request, response, router);
		if (request.getMethod().compare("DELETE") == 0)
			return HttpHandlerDELETE::handleDeleteRequest(request, response, router);
	}
	catch (const HttpException &e)
	{
		handleHttpException(serverConfig, e, response);
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
