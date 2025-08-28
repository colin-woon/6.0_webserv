#include "HttpRequest.hpp"

// OCF
HttpRequest::HttpRequest() : _method(""),
							 _target(""),
							 _path(""),
							 _queryParams(),
							 _version(""),
							 _headers(),
							 _body("")
{
}

HttpRequest::HttpRequest(const HttpRequest &other) : _method(other._method),
													 _target(other._target),
													 _path(other._path),
													 _queryParams(other._queryParams),
													 _version(other._version),
													 _headers(other._headers),
													 _body(other._body)
{
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other)
{
	if (this != &other)
	{
		_method = other._method;
		_target = other._target;
		_path = other._path;
		_queryParams = other._queryParams;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

HttpRequest::~HttpRequest()
{
}

// GETTERS
const std::string &HttpRequest::getMethod() const
{
	return _method;
}
const std::string &HttpRequest::getTarget() const
{
	return _target;
}
const std::string &HttpRequest::getPath() const
{
	return _path;
}
const std::vector<HttpRequest::QueryParams> HttpRequest::getQueryParams() const
{
	return _queryParams;
}
const std::string &HttpRequest::getVersion() const
{
	return _version;
}
const std::map<std::string, std::string> &HttpRequest::getHeaders() const
{
	return _headers;
}
const std::string &HttpRequest::getBody() const
{
	return _body;
}

// SETTERS
void HttpRequest::setMethod(const std::string &method)
{
	_method = method;
}
void HttpRequest::setTarget(const std::string &target)
{
	_target = target;
}
void HttpRequest::setPath(const std::string &path)
{
	_path = path;
}
void HttpRequest::addQueryParams(const std::string &key, const std::string &value)
{
	_queryParams.push_back(QueryParams(key, value));
}
void HttpRequest::setVersion(const std::string &version)
{
	_version = version;
}
void HttpRequest::addHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}
void HttpRequest::setBody(const std::string &body)
{
	_body = body;
}

HttpRequest::HttpRequest(const std::string &request) : _method(""),
													   _target(""),
													   _path(""),
													   _queryParams(),
													   _version(""),
													   _headers(),
													   _body("")
{
	parseRawRequest(request);
}

// Visualization https://chat.qwen.ai/s/c6b79ac1-d473-48b4-a34e-394bb622a11f?fev=0.0.201
// Use a string stream to process the request line by line
// Parse the request line (first line)
// Remove carriage return if present
void HttpRequest::parseRawRequest(const std::string &request)
{
	std::istringstream requestStream(request);
	std::string line;

	parseRequestLine(requestStream, line);
	parseHeaders(requestStream, line);
	parseBody(requestStream, line);
}

void HttpRequest::parseBody(std::istringstream &requestStream, std::string &line)
{
	std::string body;
	while (std::getline(requestStream, line))
	{
		body += line;
		if (!requestStream.eof())
			body += "\n";
	}
	_body = body;
}

void HttpRequest::parseRequestLine(std::istringstream &requestStream, std::string &line)
{
	if (std::getline(requestStream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		// Extract method, target, and version, >> is overloaded by istringstream, space is default delimeter
		std::istringstream lineStream(line);
		lineStream >> _method >> _target >> _version;

		// Parse the path and query parameters from target
		// query params start after "?"
		size_t questionPos = _target.find('?');
		if (questionPos != std::string::npos)
		{
			_path = _target.substr(0, questionPos);
			parseQueryParams(_target.substr(questionPos + 1));
		}
		else
		{
			_path = _target;
		}
	}
}

void HttpRequest::parseHeaders(std::istringstream &requestStream, std::string &line)
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
			// Skip the colon and any leading spaces
			size_t valueStart = line.find_first_not_of(" \t", colonPos + 1);
			if (valueStart != std::string::npos)
			{
				std::string value = line.substr(valueStart);
				_headers[key] = value;
			}
		}
	}
}

// Helper method to parse query parameters
void HttpRequest::parseQueryParams(const std::string &queryString)
{
	size_t start = 0;
	size_t end;

	while ((end = queryString.find('&', start)) != std::string::npos)
	{
		parseQueryParam(queryString.substr(start, end - start));
		start = end + 1;
	}
	parseQueryParam(queryString.substr(start));
}

void HttpRequest::parseQueryParam(const std::string &param)
{
	size_t equals = param.find('=');
	if (equals != std::string::npos)
	{
		std::string key = param.substr(0, equals);
		std::string value = param.substr(equals + 1);
		addQueryParams(key, value);
	}
}

std::ostream &operator<<(std::ostream &out, const HttpRequest &request)
{
	// Print the request line
	out << request.getMethod() << " " << request.getTarget() << " " << request.getVersion() << "\r\n";

	// Print headers
	const std::map<std::string, std::string> &headers = request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it)
	{
		out << it->first << ": " << it->second << "\r\n";
	}

	// Empty line to separate headers from body
	out << "\r\n";

	// Print body if it exists
	if (!request.getBody().empty())
	{
		out << request.getBody();
	}

	out << "\nQuery Parameters:\n";
	const std::vector<HttpRequest::QueryParams> &params = request.getQueryParams();
	for (std::vector<HttpRequest::QueryParams>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		out << "  " << it->first << " = " << it->second << "\n";
	}

	return out;
}
