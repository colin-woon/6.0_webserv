#ifndef HTTPEXCEPTIONS_HPP
#define HTTPEXCEPTIONS_HPP

#include <string>
#include <exception>
#include <sstream>
#include <map>
#include "HttpResponse.hpp"
#include "HttpUtils.hpp"
#include "../parsing/Server.hpp"
#include "../parsing/Location.hpp"

enum StatusCode
{
	HTTP_100_CONTINUE = 100,
	HTTP_200_OK = 200,
	HTTP_201_CREATED = 201,
	HTTP_204_NO_CONTENT = 204,
	HTTP_301_MOVED_PERMANENTLY = 301,
	HTTP_400_BAD_REQUEST = 400,
	HTTP_401_UNAUTHORIZED = 401,
	HTTP_403_FORBIDDEN = 403,
	HTTP_404_NOT_FOUND = 404,
	HTTP_405_METHOD_NOT_ALLOWED = 405,
	HTTP_413_PAYLOAD_TOO_LARGE = 413,
	HTTP_414_URI_TOO_LONG = 414,
	HTTP_415_UNSUPPORTED_MEDIA_TYPE = 415,
	HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	HTTP_500_INTERNAL_SERVER_ERROR = 500,
	HTTP_501_NOT_IMPLEMENTED = 501,
	HTTP_502_BAD_GATEWAY = 502,
	HTTP_503_SERVICE_UNAVAILABLE = 503,
	HTTP_504_GATEWAY_TIMEOUT = 504,
	HTTP_505_HTTP_VERSION_NOT_SUPPORTED = 505,
	UNKNOWN = 0
};

class HttpException : public std::exception
{
	public:
	virtual ~HttpException() throw() {};
	virtual const char *what() const throw() = 0;
	virtual const std::string getStatusCodeString() const = 0;
	virtual int getStatusCodeDigit() const = 0;
	static std::string statusCodeToString(int statusCode);
	static HttpException* createFromStatusCode(const std::string& statusCode);

};

void handleHttpException(const Server &serverConfig, const Location *locationConfig, const HttpException &e, HttpResponse &response);

// Specific HTTP exceptions
class Http400BadRequestException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "400"; };
	int getStatusCodeDigit() const { return 400; };
	const char *what() const throw();
};

class Http401UnauthorizedException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "401"; };
	int getStatusCodeDigit() const { return 401; };
	const char *what() const throw();
};

class Http403ForbiddenException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "403"; };
	int getStatusCodeDigit() const { return 403; };
	const char *what() const throw();
};

class Http404NotFoundException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "404"; };
	int getStatusCodeDigit() const { return 404; };
	const char *what() const throw();
};

class Http405MethodNotAllowedException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "405"; };
	int getStatusCodeDigit() const { return 405; };
	const char *what() const throw();
};

class Http413PayloadTooLargeException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "413"; };
	int getStatusCodeDigit() const { return 413; };
	const char *what() const throw();
};

class Http414UriTooLongException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "414"; };
	int getStatusCodeDigit() const { return 414; };
	const char *what() const throw();
};

class Http415UnsupportedMediaTypeException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "415"; };
	int getStatusCodeDigit() const { return 415; };
	const char *what() const throw();
};

class Http431RequestHeaderFieldsTooLargeException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "431"; };
	int getStatusCodeDigit() const { return 431; };
	const char *what() const throw();
};

class Http500InternalServerErrorException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "500"; };
	int getStatusCodeDigit() const { return 500; };
	const char *what() const throw();
};

class Http501NotImplementedException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "501"; };
	int getStatusCodeDigit() const { return 501; };
	const char *what() const throw();
};

class Http502BadGatewayException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "502"; };
	int getStatusCodeDigit() const { return 502; };
	const char *what() const throw();
};

class Http504GatewayTimeoutException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "504"; };
	int getStatusCodeDigit() const { return 504; };
	const char *what() const throw();
};

class Http505HttpVersionNotSupportedException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "505"; };
	int getStatusCodeDigit() const { return 505; };
	const char *what() const throw();
};

#endif
