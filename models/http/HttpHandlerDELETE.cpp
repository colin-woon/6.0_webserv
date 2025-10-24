#include "HttpHandlerDELETE.hpp"
#include <string>

HttpHandlerDELETE::HttpHandlerDELETE() {}

HttpHandlerDELETE::HttpHandlerDELETE(const HttpHandlerDELETE &other)
{
	(void)other;
}

HttpHandlerDELETE &HttpHandlerDELETE::operator=(const HttpHandlerDELETE &other)
{
	(void)other;
	return *this;
}

HttpHandlerDELETE::~HttpHandlerDELETE() {}

void HttpHandlerDELETE::handleDeleteRequest(HttpRequest &request, HttpResponse &response, Router &router)
{
	std::string path = request.getPath();

	if (path.find("/uploads/") != 0)
		throw Http404NotFoundException();

	// Extract the hashed filename from the path
	std::string hashedFilename = path.substr(9); // Remove "/uploads/" prefix

	if (hashedFilename.empty())
		throw Http400BadRequestException();

	// Delete the file (this also deletes metadata)
	FileHandler::deleteFile(hashedFilename, router, request.getCookie());

	// Set successful response
	response.setStatusCode(HttpException::statusCodeToString(HTTP_204_NO_CONTENT));
	response.addHeader("Content-Type", "text/plain");
	response.addHeader("Content-Length", "0");
	response.setBody("");
	std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Connection");
	if (it != request.getHeaders().end() && it->second == "close")
		response.addHeader("Connection", "close");
	else
		response.addHeader("Connection", "keep-alive");
}
