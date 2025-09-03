#include "HttpHandler.hpp"

// OCF
HttpHandler::HttpHandler() {}

HttpHandler::HttpHandler(const HttpHandler &other)
{
	// this->value = other.value;
}

HttpHandler &HttpHandler::operator=(const HttpHandler &other)
{
	if (this != &other)
	{
		// this->value = other.value;
	}
	return *this;
}

HttpHandler::~HttpHandler() {}

HttpResponse &HttpHandler::handleRequest(HttpRequest &request)
{

	std::string method = request.getMethod();

	std::cout << "Method size: " << method.size() << std::endl;
	std::cout << "Method data: ";
	for (size_t i = 0; i <= method.size(); ++i)
	{
		std::cout << (int)method[i] << " ";
	}
	std::cout << std::endl;

	if (method.compare("GET") == 0)
		return handleGetRequest(request);
}

HttpResponse &HttpHandler::handleGetRequest(HttpRequest &request)
{
	static HttpResponse resp;
	std::ifstream file("/home/colin/42_core_program/6.0_webserv/http/var/www/index.html", std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		resp.setStatusCode(HTTP_400_BAD_REQUEST); // Using integer directly, assuming StatusCode is defined as enum or const int
		resp.setBody("404 Not Found");
	}
	else
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();
		file.close();
		resp.setStatusCode(HTTP_200_OK); // Using integer directly, assuming StatusCode is defined as enum or const int
		resp.addHeader("Content-Type", "text/html");
		// C++98 compliant way to convert length to string
		std::stringstream lengthStream;
		lengthStream << content.length();
		std::string contentLength = lengthStream.str();
		resp.addHeader("Content-Length", contentLength);
		resp.setBody(content);
	}
	return resp;
}
