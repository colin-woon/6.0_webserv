#ifndef COOKIES_HPP
#define COOKIES_HPP

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "HttpExceptions.hpp"
#include "HttpResponse.hpp"

class Cookie
{
private:
	Cookie();
	Cookie(const Cookie &other);
	Cookie &operator=(const Cookie &other);
	~Cookie();
	static std::string createSessionID();

public:
	static std::map<std::string, std::vector<std::string> > sessionMetadata;
	static void setSecureCookie(HttpResponse &response);
	static void addHashedFileToSession(const std::string &sessionId, std::string &fileHash);
	static void removeHashedFileFromSession(const std::string &sessionId, std::string &fileHash);
	static void validateSession(const std::string &sessionId);
};

#endif
