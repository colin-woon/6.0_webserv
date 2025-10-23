#include "HttpResponse.hpp"

// OCF
HttpResponse::HttpResponse() : _statusCode(""),
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
const std::string &HttpResponse::getStatusCode() const
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
void HttpResponse::setStatusCode(const std::string &statusCode)
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
void HttpResponse::addHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

std::string HttpResponse::toString() const
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << _statusCode << " " << _statusText << "\r\n"; // Convert status code to string

	// Use C++98 iterator for map
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}

	oss << "\r\n"
		<< _body;
	return oss.str();
}

void HttpResponse::createResponse(std::string statusCode,
								  std::string statusText,
								  std::map<std::string, std::string> headers,
								  std::string body)
{
	_statusCode = statusCode;
	_statusText = statusText;
	_headers = headers;
	_body = body;
}

void HttpResponse::clear()
{
	_statusCode.clear();
	_statusText.clear();
	_headers.clear();
	_body.clear();
}
