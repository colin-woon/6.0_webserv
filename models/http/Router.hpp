#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include "../parsing/Location.hpp"
#include "../parsing/Server.hpp"
#include "HttpRequest.hpp"
#include "HttpExceptions.hpp"

class Router
{
public:
	Router();
	Router(const Router &other);
	Router &operator=(const Router &other);
	~Router();

	std::string resolvedPath;
	const Location *locationConfig;

	void getLocationConfig(HttpRequest &request, const Server &serverConfig);
	void getResolvedPath(HttpRequest &request, const Server &serverConfig);
};

#endif
