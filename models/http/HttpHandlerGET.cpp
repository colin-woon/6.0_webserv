#include "HttpHandlerGET.hpp"

std::map<std::string, std::vector<std::string> > Cookie::sessionMetadata;

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

static std::string generateAutoindexPage(const std::string &directoryPath, const std::string &requestPath)
{
	std::stringstream ss;
	ss << "<!DOCTYPE html><html><head><title>Index of " << requestPath << "</title></head><body><h1>Index of " << requestPath << "</h1><ul>";
	DIR *dir = opendir(directoryPath.c_str());
	if (dir)
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			if (name == ".")
				continue;
			std::string fullRequestPath = requestPath;
			if (fullRequestPath[fullRequestPath.size() - 1] != '/')
				fullRequestPath += "/";
			ss << "<li><a href=\"" << fullRequestPath << name << "\">" << name << "</a></li>";
		}
		closedir(dir);
	}
	ss << "</ul><hr><p><em>42_Webserv</em></p></body></html>";
	return ss.str();
}

static void handleAutoindex(Router &router, HttpRequest &request, HttpResponse &response)
{
	std::string body = generateAutoindexPage(router.resolvedPath, request.getPath());
	response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK));
	response.setBody(body);
	response.addHeader("Content-Type", "text/html");

	std::stringstream contentLength;
	contentLength << response.getBody().length();
	response.addHeader("Content-Length", contentLength.str());
	response.addHeader("Connection", "keep-alive");
}

static void serveFile(std::ifstream &file, HttpRequest &request, HttpResponse &response)
{
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();

	response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK));
	response.addHeader("Content-Type", "text/html");

	std::stringstream contentLength;
	contentLength << content.length();
	response.addHeader("Content-Length", contentLength.str());
	response.setBody(content);

	std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Connection");
	if (it != request.getHeaders().end() && it->second == "close")
		response.addHeader("Connection", "close");
	else
		response.addHeader("Connection", "keep-alive");
}

static std::string getIndexFile(Router &router, const Server &serverConfig)
{
	if (router.locationConfig && !router.locationConfig->index.empty())
		return (router.locationConfig->index);
	else
		return (serverConfig.index);
}

void HttpHandlerGET::handleGetRequest(HttpRequest &request, HttpResponse &response, Router &router, const Server &serverConfig)
{
	std::string fullPath;
	std::string pathToServe = router.resolvedPath;
	std::string sessionId;

	if (request.getCookie().empty())
	{
		sessionId = Cookie::createSessionID();
		response.addHeader("Set-Cookie", "sessionId=" + sessionId);
	}

	struct stat path_stat;
	if (stat(pathToServe.c_str(), &path_stat) != 0)
		throw Http404NotFoundException();

	if (S_ISDIR(path_stat.st_mode))
	{
		std::string indexFile = getIndexFile(router, serverConfig);
		if (pathToServe[pathToServe.size() - 1] != '/')
			pathToServe += "/";
		fullPath = pathToServe + indexFile;
		struct stat index_stat;

		if (stat(fullPath.c_str(), &index_stat) != 0 || !S_ISREG(index_stat.st_mode))
		{
			if ((router.locationConfig && router.locationConfig->autoindex) || serverConfig.autoindex)
				return handleAutoindex(router, request, response);
			else
				throw Http403ForbiddenException();
		}
	}
	else if (!S_ISREG(path_stat.st_mode))
		throw Http403ForbiddenException();
	else
		fullPath = router.resolvedPath;

	std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw Http403ForbiddenException();
	else
		return serveFile(file, request, response);
}

static void getFileListJson(std::stringstream &json, std::string &sessionId, std::vector<std::string> &sessionHashedFilenames)
{
	json << "{";
	json << "\"sessionId\":\"" << sessionId << "\",";
	json << "\"files\":[";

	bool first = true;
	for (size_t i = 0; i < sessionHashedFilenames.size(); ++i)
	{
		std::string hash = sessionHashedFilenames[i];

		// Get metadata for this file from FileHandler
		Headers metadata = FileHandler::getFileMetaData(hash);

		// Extract original filename from metadata
		std::map<std::string, std::string>::iterator nameIt = metadata.find("original-file-name");
		if (nameIt == metadata.end())
			continue; // Skip if no metadata found

		if (!first)
			json << ",";
		first = false;

		std::string originalFilename = nameIt->second;

		// Build file object
		json << "{";
		json << "\"hash\":\"" << hash << "\",";
		json << "\"original_filename\":\"" << originalFilename << "\"";

		// Add other metadata if available
		std::map<std::string, std::string>::iterator sizeIt = metadata.find("size");
		if (sizeIt != metadata.end())
			json << ",\"size\":" << sizeIt->second;

		std::map<std::string, std::string>::iterator dateIt = metadata.find("upload-date");
		if (dateIt != metadata.end())
			json << ",\"upload_date\":\"" << dateIt->second << "\"";

		json << "}";
	}

	json << "]";
	json << "}";
}

void HttpHandlerGET::handleGetRequestUploads(HttpRequest &request, HttpResponse &response, Router &router)
{
	(void)router;
	std::vector<std::string> sessionHashedFilenames;
	std::string sessionId = request.getCookie();

	if (!sessionId.empty())
		sessionHashedFilenames = Cookie::sessionMetadata[sessionId];
	else
		throw Http401UnauthorizedException();

	// Build JSON response
	std::stringstream json;
	getFileListJson(json, sessionId, sessionHashedFilenames);

	// Set response
	std::string jsonContent = json.str();
	response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK));
	response.addHeader("Content-Type", "application/json");

	std::stringstream contentLength;
	contentLength << jsonContent.length();
	response.addHeader("Content-Length", contentLength.str());
	response.addHeader("Connection", "keep-alive");
	response.setBody(jsonContent);
	std::cout << response.getBody() << std::endl;
}
