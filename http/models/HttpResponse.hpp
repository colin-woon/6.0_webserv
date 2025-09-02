#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <HttpStatusCodes.hpp>

class HttpResponse
{
private:
	StatusCode _statusCode;
	std::string _statusText;
	std::map<std::string, std::string> _headers;
	std::string _body;

public:
	HttpResponse();
	HttpResponse(const HttpResponse &other);
	HttpResponse &operator=(const HttpResponse &other);
	~HttpResponse();

	const StatusCode &getStatusCode() const;
	const std::string &getStatusText() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getBody() const;

	void setStatusCode(const StatusCode &statusCode);
	void setStatusText(const std::string &statusText);
	void setHeaders(const std::map<std::string, std::string> &headers);
	void setBody(const std::string &body);
};

#endif
