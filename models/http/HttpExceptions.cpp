#include "HttpExceptions.hpp"

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
		response.addHeader("Date", HttpUtils::getCurrentHttpDate());

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
	response.addHeader("Date", HttpUtils::getCurrentHttpDate());

	std::stringstream contentLength;
	contentLength << response.getBody().length();
	response.addHeader("Content-Length", contentLength.str());
}

void handleHttpException(const Server &serverConfig, const Location *locationConfig, const HttpException &e, HttpResponse &response)
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

std::string HttpException::statusCodeToString(int statusCode)
{
	// Simple mapping, expand as needed
	switch (statusCode)
	{
	case HTTP_200_OK:
		return "200";
	case HTTP_201_CREATED:
		return "201";
	case HTTP_204_NO_CONTENT:
		return "204";
	case HTTP_301_MOVED_PERMANENTLY:
		return "301";
	case HTTP_400_BAD_REQUEST:
		return "400";
	case HTTP_401_UNAUTHORIZED:
		return "401";
	case HTTP_404_NOT_FOUND:
		return "404";
	case HTTP_405_METHOD_NOT_ALLOWED:
		return "405";
	case HTTP_413_PAYLOAD_TOO_LARGE:
		return "413";
	case HTTP_414_URI_TOO_LONG:
		return "414";
	case HTTP_500_INTERNAL_SERVER_ERROR:
		return "500";
	case HTTP_501_NOT_IMPLEMENTED:
		return "501";
	case HTTP_502_BAD_GATEWAY:
		return "502";
	case HTTP_504_GATEWAY_TIMEOUT:
		return "504";
	case HTTP_505_HTTP_VERSION_NOT_SUPPORTED:
		return "505";
	default:
		return "503";
	}
}

HttpException* HttpException::createFromStatusCode(const std::string& statusCode)
{
	int code = std::atoi(statusCode.c_str());

	if (code == 400) return new Http400BadRequestException();
	if (code == 401) return new Http401UnauthorizedException();
	if (code == 403) return new Http403ForbiddenException();
	if (code == 404) return new Http404NotFoundException();
	if (code == 405) return new Http405MethodNotAllowedException();
	if (code == 413) return new Http413PayloadTooLargeException();
	if (code == 414) return new Http414UriTooLongException();
	if (code == 415) return new Http415UnsupportedMediaTypeException();
	if (code == 501) return new Http501NotImplementedException();
	if (code == 502) return new Http502BadGatewayException();
	if (code == 504) return new Http504GatewayTimeoutException();
	if (code == 505) return new Http505HttpVersionNotSupportedException();

	// Default to 500 for unknown errors
	return new Http500InternalServerErrorException();
}

const char *Http400BadRequestException::what() const throw()
{
	return "Bad Request";
}

const char *Http401UnauthorizedException::what() const throw()
{
	return "Unauthorized";
}

const char *Http403ForbiddenException::what() const throw()
{
	return "Forbidden";
}

const char *Http404NotFoundException::what() const throw()
{
	return "Not Found";
}

const char *Http405MethodNotAllowedException::what() const throw()
{
	return "Method Not Allowed";
}

const char *Http413PayloadTooLargeException::what() const throw()
{
	return "Payload Too Large";
}

const char *Http414UriTooLongException::what() const throw()
{
	return "URI Too Long";
}

const char *Http415UnsupportedMediaTypeException::what() const throw()
{
	return "Unsupported Media Type. Expected 'multipart/form-data'.";
}

const char *Http431RequestHeaderFieldsTooLargeException::what() const throw()
{
	return "Request Header Fields Too Large";
}

const char *Http500InternalServerErrorException::what() const throw()
{
	return "Internal Server Error";
}

const char *Http501NotImplementedException::what() const throw()
{
	return "Not Implemented";
}

const char *Http502BadGatewayException::what() const throw()
{
	return "Bad Gateway";
}

const char *Http504GatewayTimeoutException::what() const throw()
{
	return "Gateway Timeout";
}

const char *Http505HttpVersionNotSupportedException::what() const throw()
{
	return "Http Version Not Supported";
}
