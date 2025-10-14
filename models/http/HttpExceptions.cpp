#include "HttpExceptions.hpp"

std::string HttpException::statusCodeToString(int statusCode)
{
	// Simple mapping, expand as needed
	switch (statusCode)
	{
	case HTTP_200_OK:
		return "200";
	case HTTP_204_NO_CONTENT:
		return "204";
	case HTTP_400_BAD_REQUEST:
		return "400";
	case HTTP_404_NOT_FOUND:
		return "404";
	case HTTP_405_METHOD_NOT_ALLOWED:
		return "405";
	case HTTP_413_PAYLOAD_TOO_LARGE:
		return "413";
	case HTTP_414_URI_TOO_LONG:
		return "414";
	case HTTP_500_INTERNAL_SERVER_ERROR:
		return "500";
	case HTTP_501_NOT_IMPLEMENTED:
		return "501";
	case HTTP_505_HTTP_VERSION_NOT_SUPPORTED:
		return "505";
	default:
		return "503";
	}
}

const char *Http400BadRequestException::what() const throw()
{
	return "Bad Request";
}

const char *Http404NotFoundException::what() const throw()
{
	return "Not Found";
}

const char *Http405MethodNotAllowedException::what() const throw()
{
	return "Method Not Allowed";
}

const char *Http413PayloadTooLargeException::what() const throw()
{
	return "Payload Too Large";
}

const char *Http414UriTooLongException::what() const throw()
{
	return "URI Too Long";
}

const char *Http500InternalServerErrorException::what() const throw()
{
	return "Internal Server Error";
}

const char *Http501NotImplementedException::what() const throw()
{
	return "Not Implemented";
}

const char *Http505HttpVersionNotSupportedException::what() const throw()
{
	return "Http Version Not Supported";
}
