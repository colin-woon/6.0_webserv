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

class Cookies
{
private:
	Cookies();
	Cookies(const Cookies &other);
	Cookies &operator=(const Cookies &other);
	~Cookies();

public:
	static std::map<std::string, std::vector<std::string> > sessionMetadata;
	static std::string createSessionID();
};

#endif
