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
	std::string body;
	while (std::getline(requestStream, line))
	{
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

static void parseRequestLine(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	if (std::getline(requestStream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		// Extract method, target, and version, >> is overloaded by istringstream, space is default delimeter
		std::istringstream lineStream(line);
		std::string method, target, version;
		lineStream >> method >> target >> version;

		request.setMethod(method);
		request.setTarget(target);
		request.setVersion(version);

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
}

static void parseHeaders(HttpRequest &request, std::istringstream &requestStream, std::string &line)
{
	while (std::getline(requestStream, line) && !line.empty() && line != "\r")
	{
		// Remove carriage return if present
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		// Skip empty lines
		if (line.empty())
			continue;

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

			// Validate: No remaining \r should exist (indicates invalid CR not followed by LF)
			if (line.find('\r') != std::string::npos)
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
}
