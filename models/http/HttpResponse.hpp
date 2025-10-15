#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sstream>

class HttpResponse
{
private:
	std::string _statusCode;
	std::string _statusText;
	std::map<std::string, std::string> _headers;
	std::string _body;

public:
	HttpResponse();
	HttpResponse(const HttpResponse &other);
	HttpResponse &operator=(const HttpResponse &other);
	~HttpResponse();

	const std::string &getStatusCode() const;
	const std::string &getStatusText() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getBody() const;

	void setStatusCode(const std::string &statusCode);
	void setStatusText(const std::string &statusText);
	void setHeaders(const std::map<std::string, std::string> &headers);
	void setBody(const std::string &body);

	void addHeader(const std::string &key, const std::string &value);
	std::string toString() const;
	std::string statusToString(int status) const;

	void clear();

	void createResponse(std::string _statusCode,
						std::string _statusText,
						std::map<std::string, std::string> _headers,
						std::string _body);
};

#endif
