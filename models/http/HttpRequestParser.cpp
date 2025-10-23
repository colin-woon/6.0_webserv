#include "HttpRequestParser.hpp"
#include "HttpRequest.hpp"
#include "HttpExceptions.hpp"
#include <cstdlib>

static void parseBody(HttpRequest &request, std::istringstream &requestStream, std::string &line, const Server &serverConfig)
{
	const std::map<std::string, std::string> &headers = request.getHeaders();
	std::string body;

	if (request.getMethod().compare("GET") == 0 || request.getMethod().compare("DELETE") == 0)
	{
		HttpUtils::skipEmptyLines(requestStream, line);
		if (!requestStream.eof())
			throw Http400BadRequestException();
		else
			return;
	}

	if (headers.find("Content-Length") != headers.end() && headers.find("Transfer-Encoding") != headers.end())
		throw Http400BadRequestException();

	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
	if (it != headers.end())
	{
		const std::string &contentLengthStr = it->second;
		char *endptr;
		long contentLength = std::strtol(contentLengthStr.c_str(), &endptr, 10);

		if (*endptr != '\0' || contentLength < 0)
			throw Http400BadRequestException();

		if (contentLength > serverConfig.max_body_size)
			throw Http413PayloadTooLargeException();

		body.resize(contentLength);
		if (contentLength > 0)
		{
			requestStream.read(&body[0], contentLength);
			// std::cout << "Expected Content-Length: " << contentLength << std::endl;
			// std::cout << "Bytes actually read (gcount): " << requestStream.gcount() << std::endl;
			if (requestStream.gcount() != contentLength)
				throw Http400BadRequestException();
		}
	}
	else if (headers.find("Transfer-Encoding") != headers.end())
	{
		std::cout << "TEMP NOT YET IMPLEMENT TRANSFER ENCODING" << std::endl;
		if (headers.find("Transfer-Encoding")->second == "chunked")
			throw Http501NotImplementedException();
	}
	else if (request.getMethod().compare("POST") == 0)
	{
		throw Http400BadRequestException();
	}
	request.setBody(body);
}

static void parseRequestLine(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	HttpUtils::skipEmptyLines(requestStream, line);
	HttpUtils::checkForNUL(line);
	HttpUtils::processCarriageReturn(line);
	HttpUtils::getMethodTargetVersion(request, line);
	HttpUtils::parseTarget(request);
}

static void parseHeaders(HttpRequest &request, std::istringstream &requestStream, std::string &line,
						 const Server &serverConfig)
{
	size_t totalHeaderSize = 0;
	while (std::getline(requestStream, line))
	{
		totalHeaderSize += line.length() + 1;
		if (totalHeaderSize > static_cast<size_t>(serverConfig.header_cap))
			throw Http431RequestHeaderFieldsTooLargeException();

		HttpUtils::checkForNUL(line);
		if (line.size() == 0 || (line.size() == 1 && line[0] == '\r'))
		{
			HttpUtils::handleEndOfHeaders(request, requestStream);
			return;
		}
		HttpUtils::processCarriageReturn(line);
		HttpUtils::parseHeaderKeyValuePair(line, request);
	}
}

static void postParsingValidation(HttpRequest &request)
{
	const int maxMethodSize = 10;
	std::set<std::string> implementedMethods;
	implementedMethods.insert("GET");
	implementedMethods.insert("POST");
	implementedMethods.insert("DELETE");
	char uppercaseLetters[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	if (request.getMethod().find_first_not_of(uppercaseLetters) != std::string::npos)
		throw Http400BadRequestException();
	if (request.getMethod().empty() ||
		request.getTarget().empty() ||
		request.getVersion().empty())
		throw Http400BadRequestException();
	if (implementedMethods.find(request.getMethod()) == implementedMethods.end() ||
		request.getMethod().size() > maxMethodSize)
		throw Http501NotImplementedException();
	if (request.getVersion().compare("HTTP/1.1"))
		throw Http505HttpVersionNotSupportedException();
}

// Visualization https://chat.qwen.ai/s/c6b79ac1-d473-48b4-a34e-394bb622a11f?fev=0.0.201
// Use a string stream to process the request line by line
// Parse the request line (first line)
// Remove carriage return if present
void HttpRequestParser::parseRawRequest(HttpRequest &request, const std::string &rawRequestBytes,
										const Server &serverConfig)
{
	std::istringstream requestStream(rawRequestBytes);
	std::string line;

	parseRequestLine(request, requestStream, line);
	parseHeaders(request, requestStream, line, serverConfig);
	parseBody(request, requestStream, line, serverConfig);
	postParsingValidation(request);
}
