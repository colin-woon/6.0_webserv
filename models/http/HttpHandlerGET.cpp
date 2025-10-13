#include "HttpHandlerGET.hpp"

HttpHandlerGET::HttpHandlerGET() {}

HttpHandlerGET::HttpHandlerGET(const HttpHandlerGET &other)
{
	(void)other;
}

HttpHandlerGET &HttpHandlerGET::operator=(const HttpHandlerGET &other)
{
	(void)other;
	return *this;
}

HttpHandlerGET::~HttpHandlerGET() {}

static std::string generateFileListJson()
{
	std::map<std::string, Headers> allFiles = FileHandler::getAllFileMetaData();
	std::string json = "[";
	bool first = true;

	for (std::map<std::string, Headers>::iterator it = allFiles.begin();
		 it != allFiles.end(); ++it)
	{
		if (!first)
			json += ",";
		first = false;

		std::string hash = it->first;
		std::map<std::string, std::string> metadata = it->second;

		// Check if original-file-name exists in metadata
		std::map<std::string, std::string>::iterator nameIt = metadata.find("original-file-name");
		std::string originalFilename = (nameIt != metadata.end()) ? nameIt->second : "unknown";

		json += "{";
		json += "\"hash\":\"" + hash + "\",";
		json += "\"original_filename\":\"" + originalFilename + "\"";
		json += "}";
	}

	json += "]";
	return json;
}

void HttpHandlerGET::handleGetRequest(HttpRequest &request, HttpResponse &response, Router &router)
{
	// std::string TEMP_root = "/home/colin/42_core_program/6.0_webserv/var/www";

	// std::string path = request.getPath();

	// // Handle file list endpoint
	// if (path.compare("/files") == 0)
	// {
	// 	std::string jsonContent = generateFileListJson();

	// 	response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK));
	// 	response.addHeader("Content-Type", "application/json");

	// 	std::stringstream contentLength;
	// 	contentLength << jsonContent.length();
	// 	response.addHeader("Content-Length", contentLength.str());
	// 	response.setBody(jsonContent);
	// 	return;
	// }

	// if (path.compare("/") == 0)
	// 	path = "/index.html";
	std::string fullPath = router.resolvedPath;

	std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw Http404NotFoundException();
	else
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();
		file.close();

		response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK)); // Using integer directly, assuming StatusCode is defined as enum or const int
		response.addHeader("Content-Type", "text/html");

		std::stringstream contentLength;
		contentLength << content.length();
		response.addHeader("Content-Length", contentLength.str());
		response.setBody(content);
	}
}
