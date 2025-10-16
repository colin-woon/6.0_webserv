#include "HttpUtils.hpp"
#include "HttpRequest.hpp"

HttpUtils::HttpUtils() {}

HttpUtils::HttpUtils(const HttpUtils &other)
{
	(void)other;
}

HttpUtils &HttpUtils::operator=(const HttpUtils &other)
{
	(void)other;
	return *this;
}

HttpUtils::~HttpUtils() {}

void HttpUtils::skipEmptyLines(std::istringstream &requestStream, std::string &line)
{
	while (std::getline(requestStream, line))
	{
		if ((line.size() == 1 && line[0] == '\r') || line.size() == 0)
			continue;
		break;
	}
}

void HttpUtils::checkForNUL(std::string &line)
{
	size_t nulPos = line.find('\0');
	if (nulPos != std::string::npos)
		throw Http400BadRequestException();
}

void HttpUtils::processCarriageReturn(std::string &line)
{
	size_t crPos = line.find('\r');
	if (crPos != std::string::npos)
	{
		if (crPos != line.size() - 1)
			throw Http400BadRequestException();
	}
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
}

static void parseQueryParam(HttpRequest &request, const std::string &param)
{
	size_t equals = param.find('=');
	if (equals != std::string::npos)
	{
		std::string key = param.substr(0, equals);
		std::string value = param.substr(equals + 1);
		request.addQueryParams(key, value);
	}
}

// Helper method to parse query parameters
static void parseQueryParams(HttpRequest &request, const std::string &queryString)
{
	size_t start = 0;
	size_t end;

	while ((end = queryString.find('&', start)) != std::string::npos)
	{
		parseQueryParam(request, queryString.substr(start, end - start));
		start = end + 1;
	}
	parseQueryParam(request, queryString.substr(start));
}

void HttpUtils::getMethodTargetVersion(HttpRequest &request, std::string &line)
{
	// Extract method, target, and version, >> is overloaded by istringstream, space is default delimeter
	std::istringstream lineStream(line);
	std::string method, target, version;
	lineStream >> method >> target >> version;

	if (!lineStream.eof())
		throw Http400BadRequestException();

	request.setMethod(method);
	request.setTarget(target);
	request.setVersion(version);
}

void HttpUtils::parseTarget(HttpRequest &request)
{
	if (request.getTarget().size() > HttpUtils::MAX_URI_LENGTH)
		throw Http414UriTooLongException();

	// Parse the path and query parameters from target
	// query params start after "?"
	std::string rawTarget = request.getTarget();
	size_t questionPos = rawTarget.find('?');
	if (questionPos != std::string::npos)
	{
		request.setPath(rawTarget.substr(0, questionPos));
		parseQueryParams(request, rawTarget.substr(questionPos + 1));
	}
	else
		request.setPath(rawTarget);
}

void HttpUtils::handleEndOfHeaders(HttpRequest &request, std::istringstream &requestStream)
{
	const std::map<std::string, std::string> &headers = request.getHeaders();

	if (headers.empty())
		throw Http400BadRequestException();
	else if (headers.count("Host") != 1)
		throw Http400BadRequestException();
	else
	{
		std::string nextLine;
		std::streampos pos = requestStream.tellg();
		std::istringstream requestStreamCopy(requestStream.str());
		if (pos != static_cast<std::streampos>(-1))
			requestStreamCopy.seekg(pos);
		std::getline(requestStreamCopy, nextLine);
		char nextCharInRequestStream = requestStream.peek();

		if (request.getMethod().compare("GET"))
		{
			if ((nextLine.size() == 0 && nextCharInRequestStream != EOF) || (nextLine.size() == 1 && nextLine[0] == '\r' && nextCharInRequestStream != EOF))
				throw Http400BadRequestException();
		}
	}
}

void HttpUtils::parseHeaderKeyValuePair(std::string &line, HttpRequest &request)
{
	const std::map<std::string, std::string> &headers = request.getHeaders();

	size_t whitespacePos = line.find_first_of(" \t");
	size_t headerFieldPos = line.find_first_not_of(" \t");
	if (whitespacePos < headerFieldPos)
		throw Http400BadRequestException();

	// Find the colon separator
	size_t colonPos = line.find(':');
	if (colonPos != std::string::npos)
	{
		std::string key = line.substr(0, colonPos);
		if (whitespacePos < colonPos)
			throw Http400BadRequestException();
		// Trim leading whitespace from key (C++98 compatible)
		size_t start = key.find_first_not_of(" \t");
		if (start != std::string::npos)
			key = key.substr(start);
		else
			key.clear(); // All whitespace, make empty

		if (headers.find("Host") != headers.end() && key.compare("Host") == 0)
			throw Http400BadRequestException();

		size_t valueStart = line.find_first_not_of(" \t", colonPos + 1);
		if (valueStart != std::string::npos)
		{
			std::string value = line.substr(valueStart);
			request.addHeader(key, value);
		}
	}
}
