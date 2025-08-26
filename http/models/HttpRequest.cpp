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

void HttpRequest::parseRawRequest(const std::string &request)
{
	std::cout << request << std::endl;
}
