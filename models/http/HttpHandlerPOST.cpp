#include "HttpHandlerPOST.hpp"

HttpHandlerPOST::HttpHandlerPOST() {}

HttpHandlerPOST::HttpHandlerPOST(const HttpHandlerPOST &other)
{
	(void)other;
}

HttpHandlerPOST &HttpHandlerPOST::operator=(const HttpHandlerPOST &other)
{
	(void)other;
	return *this;
}

HttpHandlerPOST::~HttpHandlerPOST() {}

static unsigned long djb2Hash(const std::string &str)
{
	unsigned long hash = 5381;
	for (size_t i = 0; i < str.size(); ++i)
	{
		hash = ((hash << 5) + hash) + str[i]; // hash * 33 + c
	}
	return hash;
}

static std::string toHexString(unsigned long num)
{
	std::stringstream ss;
	ss << std::hex << num;
	return ss.str();
}

// static void parseFileHeaderKeyValuePair(std::string &line, std::map<std::string, std::string> fileHeaders)
// {
// 	size_t whitespacePos = line.find_first_of(" \t");
// 	size_t headerFieldPos = line.find_first_not_of(" \t");
// 	if (whitespacePos < headerFieldPos)
// 		throw Http400BadRequestException();

// 	// Find the colon separator
// 	size_t colonPos = line.find(':');
// 	if (colonPos != std::string::npos)
// 	{
// 		std::string key = line.substr(0, colonPos);
// 		if (whitespacePos < colonPos)
// 			throw Http400BadRequestException();
// 		// Trim leading whitespace from key (C++98 compatible)
// 		size_t start = key.find_first_not_of(" \t");
// 		if (start != std::string::npos)
// 			key = key.substr(start);
// 		else
// 			key.clear(); // All whitespace, make empty

// 		size_t valueStart = line.find_first_not_of(" \t", colonPos + 1);
// 		if (valueStart != std::string::npos)
// 		{
// 			std::string value = line.substr(valueStart);
// 			fileHeaders[key] = value;
// 		}
// 	}
// }

static void parseFileHeaders(std::map<std::string, std::string> &fileHeaders, HttpRequest &request, std::istringstream &bodyStream)
{
	(void)request;
	std::string line;

	while (std::getline(bodyStream, line))
	{
		if (!line.empty() && line[line.size() - 1] == ' ')
			line.erase(line.size() - 1);
		if (line.empty())
			return;
		std::cout << line << std::endl;
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			// size_t start = key.find_first_not_of(" \t");
			// if (start != std::string::npos)
			// 	key = key.substr(start);
			// else
			// 	key.clear();

			size_t valueStart = line.find_first_not_of(" \t", colonPos + 1);
			if (valueStart != std::string::npos)
			{
				std::string value = line.substr(valueStart);
				fileHeaders[key] = value;
			}
		}
		// HttpUtils::checkForNUL(line);
		// if (line.size() == 0 || (line.size() == 1 && line[0] == '\r'))
		// {
		// 	HttpUtils::handleEndOfHeaders(request, bodyStream);
		// 	return;
		// }
		// HttpUtils::processCarriageReturn(line);
		// parseFileHeaderKeyValuePair(line, fileHeaders);
	}
}

static void getBoundary(std::map<std::string, std::string> &headers, std::string &boundary)
{
	std::string &contentType = headers.at("Content-Type");
	size_t equalPos = contentType.find("=");
	boundary = contentType.substr(equalPos + 1);
}

static void getFileHeadersAndContent(std::string &body, std::map<std::string, std::string> &fileHeaders, HttpRequest &request, std::string &boundary, std::string &fileContent)
{
	size_t headerMetadataEnd = body.find("\r\n\r\n");
	std::string headerMetadata = body.substr(0, headerMetadataEnd);
	std::istringstream headerStream(headerMetadata);
	parseFileHeaders(fileHeaders, request, headerStream);

	std::string closingBoundary = "--" + boundary + "--";
	size_t closingPos = body.find(closingBoundary);
	if (closingPos == std::string::npos)
	{
		std::cout << "ntohign" << std::endl;
		return;
	}
	size_t dataStart = headerMetadataEnd + 4;
	fileContent = body.substr(dataStart, closingPos - dataStart);
}

static void getFileNameAndExtension(std::map<std::string, std::string> &fileHeaders, std::string &filenameValue, std::string &fileExtension)
{
	const std::string filenameKey = "filename=";
	std::string contentDisposition = fileHeaders.at("Content-Disposition");
	size_t startPos = contentDisposition.find(filenameKey);
	size_t filenameStart = startPos + filenameKey.size() + 1;
	size_t filenameLength = contentDisposition.size() - filenameStart - 2;
	filenameValue = contentDisposition.substr(filenameStart, filenameLength);

	size_t lastPeriodPos = filenameValue.find_last_of(".");
	fileExtension = filenameValue.substr(lastPeriodPos);
}

static std::string uploadHashedFile(std::string &filenameValue, std::string &fileExtension, std::string &fileContent, std::map<std::string, std::string> &fileHeaders, Router &router, const std::string &sessionId, const std::string &requestPath)
{
	unsigned long hashed = djb2Hash(filenameValue);
	std::string hashedFilename = toHexString(hashed) + fileExtension;
	std::string uploadFileLocation;

	fileHeaders["original-file-name"] = filenameValue;
	FileHandler::addNewFileMetaData(hashedFilename, fileHeaders);
	uploadFileLocation = FileHandler::uploadFile(hashedFilename, fileContent, router, requestPath);
	Cookie::addHashedFileToSession(sessionId, hashedFilename);

	std::cout << FileHandler::getFileMetaData(hashedFilename)["Content-Disposition"] << std::endl;
	return uploadFileLocation;
}

static bool isInvalidContentType(std::map<std::string, std::string> &headers)
{
	std::string contentTypeValue = headers.at("Content-Type");
	std::string mimeType;

	size_t semicolonPos = contentTypeValue.find(';');
	if (semicolonPos != std::string::npos)
		mimeType = contentTypeValue.substr(0, semicolonPos);
	else
		mimeType = contentTypeValue;

	if (mimeType != "multipart/form-data")
		return true;
	return false;
}

void HttpHandlerPOST::handlePostRequest(HttpRequest &request, HttpResponse &response, Router &router)
{
	try
	{
		std::map<std::string, std::string> headers = request.getHeaders();
		if (isInvalidContentType(headers))
			throw Http415UnsupportedMediaTypeException();

		std::string sessionId = request.getCookie();
		Cookie::validateSession(sessionId);

		std::string body = request.getBody();
		std::string boundary;
		std::map<std::string, std::string> fileHeaders;
		std::string fileContent;
		std::string uploadFileLocation;

		getBoundary(headers, boundary);
		getFileHeadersAndContent(body, fileHeaders, request, boundary, fileContent);

		std::string filenameValue;
		std::string fileExtension;
		getFileNameAndExtension(fileHeaders, filenameValue, fileExtension);

		uploadFileLocation = uploadHashedFile(filenameValue, fileExtension, fileContent, fileHeaders, router, request.getCookie(), request.getPath());

		response.setStatusCode(HttpException::statusCodeToString(HTTP_201_CREATED));
		response.addHeader("Content-Type", "text/plain");
		response.addHeader("Content-Length", "18");
		response.addHeader("Location", uploadFileLocation);
		response.setBody("Upload successful\n");
		std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Connection");
		if (it != request.getHeaders().end() && it->second == "close")
			response.addHeader("Connection", "close");
		else
			response.addHeader("Connection", "keep-alive");
	}
	catch (std::out_of_range)
	{
		throw Http400BadRequestException();
	}
}
