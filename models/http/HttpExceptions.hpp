#ifndef HTTPEXCEPTIONS_HPP
#define HTTPEXCEPTIONS_HPP

#include <string>
#include <exception>

enum StatusCode
{
	HTTP_100_CONTINUE = 100,
	HTTP_200_OK = 200,
	HTTP_201_CREATED = 201,
	HTTP_204_NO_CONTENT = 204,
	HTTP_400_BAD_REQUEST = 400,
	HTTP_401_UNAUTHORIZED = 401,
	HTTP_403_FORBIDDEN = 403,
	HTTP_404_NOT_FOUND = 404,
	HTTP_405_METHOD_NOT_ALLOWED = 405,
	HTTP_413_PAYLOAD_TOO_LARGE = 413,
	HTTP_414_URI_TOO_LONG = 414,
	HTTP_500_INTERNAL_SERVER_ERROR = 500,
	HTTP_501_NOT_IMPLEMENTED = 501,
	HTTP_502_BAD_GATEWAY = 502,
	HTTP_503_SERVICE_UNAVAILABLE = 503,
	HTTP_505_HTTP_VERSION_NOT_SUPPORTED = 505,
	UNKNOWN = 0
};

class HttpException : public std::exception
{
public:
	virtual ~HttpException() throw() {};
	virtual const char *what() const throw() = 0;
	virtual const std::string getStatusCodeString() const = 0;
	virtual const int getStatusCodeDigit() const = 0;
	static std::string statusCodeToString(int statusCode);
};

// Specific HTTP exceptions
class Http400BadRequestException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "400"; };
	const int getStatusCodeDigit() const { return 400; };
	const char *what() const throw();
};

class Http404NotFoundException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "404"; };
	const int getStatusCodeDigit() const { return 404; };
	const char *what() const throw();
};

class Http405MethodNotAllowedException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "405"; };
	const int getStatusCodeDigit() const { return 405; };
	const char *what() const throw();
};

class Http413PayloadTooLargeException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "413"; };
	const int getStatusCodeDigit() const { return 413; };
	const char *what() const throw();
};

class Http414UriTooLongException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "414"; };
	const int getStatusCodeDigit() const { return 414; };
	const char *what() const throw();
};

class Http500InternalServerErrorException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "500"; };
	const int getStatusCodeDigit() const { return 500; };
	const char *what() const throw();
};

class Http501NotImplementedException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "501"; };
	const int getStatusCodeDigit() const { return 501; };
	const char *what() const throw();
};

class Http502BadGatewayException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "502"; };
	const int getStatusCodeDigit() const { return 502; };
	const char *what() const throw();
};

class Http505HttpVersionNotSupportedException : public HttpException
{
public:
	const std::string getStatusCodeString() const { return "505"; };
	const int getStatusCodeDigit() const { return 505; };
	const char *what() const throw();
};

#endif
