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
void HttpResponse::addHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

//
std::string HttpResponse::toString() const
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << statusToString(_statusCode) << "\r\n"; // Convert status code to string

	// Use C++98 iterator for map
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}

	oss << "\r\n"
		<< _body;
	return oss.str();
}

std::string HttpResponse::statusToString(int status) const
{
	// Simple mapping, expand as needed
	switch (status)
	{
	case HTTP_200_OK:
		return "200 OK";
	case HTTP_404_NOT_FOUND:
		return "404 Not Found";
	default:
		return "500 Internal Server Error";
	}
}
