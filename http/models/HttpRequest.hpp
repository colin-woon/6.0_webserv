#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>

class HttpRequest
{
public:
	typedef std::pair<std::string, std::string> QueryParams;

private:
	std::string _method;
	std::string _target;
	std::string _path;
	std::vector<QueryParams> _queryParams;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _body;

public:
	HttpRequest();
	HttpRequest(const HttpRequest &other);
	HttpRequest &operator=(const HttpRequest &other);
	~HttpRequest();

	const std::string &getMethod() const;
	const std::string &getTarget() const;
	const std::string &getPath() const;
	const std::vector<QueryParams> getQueryParams() const;
	const std::string &getVersion() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getBody() const;

	void setMethod(const std::string &method);
	void setTarget(const std::string &target);
	void setPath(const std::string &path);
	void addQueryParams(const std::string &key, const std::string &value);
	void setVersion(const std::string &version);
	void addHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &body);

	HttpRequest(const std::string &request);
	void parseRawRequest(const std::string &request);

	void parseQueryParams(const std::string &queryString);
	void parseQueryParam(const std::string &param);
};

std::ostream &operator<<(std::ostream &out, const HttpRequest &request);

#endif
