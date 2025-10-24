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

class Cookie
{
private:
	Cookie();
	Cookie(const Cookie &other);
	Cookie &operator=(const Cookie &other);
	~Cookie();

public:
	static std::map<std::string, std::vector<std::string> > sessionMetadata;
	static std::string createSessionID();
	static void addHashedFileToSession(const std::string &sessionId, std::string &fileHash);
	static void removeHashedFileFromSession(const std::string &sessionId, std::string &fileHash);
};

#endif
