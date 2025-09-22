#include "HttpUtils.hpp"
#include "HttpRequest.hpp"
#include "HttpExceptions.hpp"

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

static void parseBody(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	const std::map<std::string, std::string> &headers = request.getHeaders();
	std::string body;

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

static void skipEmptyLines(std::istringstream &requestStream, std::string &line)
{
	while (std::getline(requestStream, line))
	{
		if (line.size() == 1 && line[0] == '\r' || line.size() == 0)
			continue;
		break;
	}
}

static void parseRequestLine(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	skipEmptyLines(requestStream, line);
	size_t crPos = line.find('\r');
	if (crPos != std::string::npos)
	{
		if (crPos != line.size() - 1)
			throw Http400BadRequestException();
	}
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	// Extract method, target, and version, >> is overloaded by istringstream, space is default delimeter
	std::istringstream lineStream(line);
	std::string method, target, version;
	lineStream >> method >> target >> version;

	request.setMethod(method);
	request.setTarget(target);
	request.setVersion(version);

	if (target.size() > HttpUtils::MAX_URI_LENGTH)
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
	{
		request.setPath(rawTarget);
	}
}

static void parseHeaders(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	const std::map<std::string, std::string> &headers = request.getHeaders();

	while (std::getline(requestStream, line))
	{
		if (line.size() == 0 || (line.size() == 1 && line[0] == '\r'))
		{
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
				int nextCharInRequestStream = requestStream.peek();

				if ((nextLine.size() == 0 && nextCharInRequestStream != EOF) || (nextLine.size() == 1 && nextLine[0] == '\r' && nextCharInRequestStream != EOF))
					throw Http400BadRequestException();
			}
			return;
		}
		size_t crPos = line.find('\r');
		if (crPos != std::string::npos)
		{
			if (crPos != line.size() - 1)
				throw Http400BadRequestException();
		}
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t whitespacePos = line.find_first_of(" \t");
		size_t headerFieldPos = line.find_first_not_of(" \t");
		if (whitespacePos < headerFieldPos)
			throw Http400BadRequestException();

		// Find the colon separator
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
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
}

static void postParseMethod(HttpRequest &request)
{
	const int maxMethodSize = 10;
	std::set<std::string> implementedMethods = {"GET", "POST", "DELETE"};
	char uppercaseLetters[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	if (request.getMethod().find_first_not_of(uppercaseLetters) != std::string::npos)
	{
		request.addHeader("Connection", "close");
		throw Http400BadRequestException();
	}
	if (implementedMethods.find(request.getMethod()) == implementedMethods.end() || request.getMethod().size() > maxMethodSize)
		throw Http501NotImplementedException();
}

// Visualization https://chat.qwen.ai/s/c6b79ac1-d473-48b4-a34e-394bb622a11f?fev=0.0.201
// Use a string stream to process the request line by line
// Parse the request line (first line)
// Remove carriage return if present
void HttpUtils::parseRawRequest(HttpRequest &request, const std::string &rawRequestBytes)
{
	std::istringstream requestStream(rawRequestBytes);
	std::string line;

	parseRequestLine(request, requestStream, line);
	parseHeaders(request, requestStream, line);
	parseBody(request, requestStream, line);
	postParseMethod(request);
}
