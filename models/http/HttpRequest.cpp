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
	HttpRequestParser::parseRawRequest(*this, request);
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
