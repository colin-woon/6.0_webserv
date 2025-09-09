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

void HttpHandler::handleRequest(HttpRequest &request, HttpResponse &response)
{
	if (request.getMethod().compare("GET") == 0)
		return handleGetRequest(request, response);
}

void HttpHandler::handleGetRequest(HttpRequest &request, HttpResponse &response)
{
	std::string TEMP_root = "/home/colin/42_core_program/6.0_webserv/http/var/www";

	std::string path = request.getPath();

	if (path.compare("/") == 0)
		path = "/index.html";
	std::string fullPath = TEMP_root + path;
	std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		response.setStatusCode(HTTP_404_NOT_FOUND); // Using integer directly, assuming StatusCode is defined as enum or const int
		response.setBody("404 Not Found");
	}
	else
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();
		file.close();

		response.setStatusCode(HTTP_200_OK); // Using integer directly, assuming StatusCode is defined as enum or const int
		response.addHeader("Content-Type", "text/html");

		std::stringstream contentLength;
		contentLength << content.length();
		response.addHeader("Content-Length", contentLength.str());
		response.setBody(content);
	}
}
