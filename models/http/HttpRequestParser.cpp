#include "HttpRequestParser.hpp"
#include "HttpRequest.hpp"
#include "HttpExceptions.hpp"

static void parseBody(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	const std::map<std::string, std::string> &headers = request.getHeaders();
	std::string body;

	if (request.getMethod().compare("GET") == 0)
	{
		HttpUtils::skipEmptyLines(requestStream, line);
		if (!requestStream.eof())
			throw Http400BadRequestException();
		else
			return;
	}
	while (std::getline(requestStream, line))
	{
		if (!line.empty() && headers.find("Content-Length") == headers.end() && headers.find("Transfer-Encoding") == headers.end())
			throw Http400BadRequestException();
		else if (headers.find("Content-Length") != headers.end() && headers.find("Transfer-Encoding") != headers.end())
			throw Http400BadRequestException();

		size_t crPos = std::string::npos;
		while (true)
		{
			crPos = line.find('\r');
			if (crPos == std::string::npos)
				break;
			else
				line[crPos] = ' ';
		}
		body += line;
		if (!requestStream.eof())
			body += "\n";
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

static void parseHeaders(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	const std::map<std::string, std::string> &headers = request.getHeaders();

	while (std::getline(requestStream, line))
	{
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
	std::set<std::string> implementedMethods = {"GET", "POST", "DELETE"};
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
void HttpRequestParser::parseRawRequest(HttpRequest &request, const std::string &rawRequestBytes)
{
	std::istringstream requestStream(rawRequestBytes);
	std::string line;

	parseRequestLine(request, requestStream, line);
	parseHeaders(request, requestStream, line);
	parseBody(request, requestStream, line);
	postParsingValidation(request);
}
