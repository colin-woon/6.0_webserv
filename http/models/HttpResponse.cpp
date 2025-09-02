#include "HttpResponse.hpp"

// OCF
HttpResponse::HttpResponse() : _statusCode(UNKNOWN),
							   _statusText(""),
							   _headers(),
							   _body("")
{
}

HttpResponse::HttpResponse(const HttpResponse &other) : _statusCode(other._statusCode),
														_statusText(other._statusText),
														_headers(other._headers),
														_body(other._body)
{
}

HttpResponse &HttpResponse::operator=(const HttpResponse &other)
{
	if (this != &other)
	{
		_statusCode = other._statusCode;
		_statusText = other._statusText;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}

// GETTER
const StatusCode &HttpResponse::getStatusCode() const
{
	return _statusCode;
}
const std::string &HttpResponse::getStatusText() const
{
	return _statusText;
}
const std::map<std::string, std::string> &HttpResponse::getHeaders() const
{
	return _headers;
}
const std::string &HttpResponse::getBody() const
{
	return _body;
}

// SETTER
void HttpResponse::setStatusCode(const StatusCode &statusCode)
{
	_statusCode = statusCode;
}
void HttpResponse::setStatusText(const std::string &statusText)
{
	_statusText = statusText;
}
void HttpResponse::setHeaders(const std::map<std::string, std::string> &headers)
{
	_headers = headers;
}
void HttpResponse::setBody(const std::string &body)
{
	_body = body;
}
//
