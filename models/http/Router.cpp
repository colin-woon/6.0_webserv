#include "Router.hpp"

Router::Router() : resolvedPath(""),
				   locationConfig(NULL)
{
}

Router::Router(const Router &other) : resolvedPath(other.resolvedPath),
									  locationConfig(other.locationConfig)
{
}

Router &Router::operator=(const Router &other)
{
	if (this != &other)
	{
		resolvedPath = other.resolvedPath;
		locationConfig = other.locationConfig;
	}
	return *this;
}

Router::~Router()
{
}

void Router::getLocationConfig(HttpRequest &request, const Server &serverConfig)
{
	const std::string &requestPath = request.getPath();
	for (size_t i = 0; i < serverConfig.location.size(); ++i)
	{
		const std::string &locationPath = serverConfig.location[i].path;

		if (requestPath.compare(0, locationPath.size(), locationPath) == 0)
		{
			locationConfig = &serverConfig.location[i];
			break;
		}
	}
}

void Router::getResolvedPath(HttpRequest &request, const Server &serverConfig)
{
	resolvedPath = serverConfig.root;
	if (locationConfig != NULL)
	{
		if (!locationConfig->root.empty())
			resolvedPath = locationConfig->root;
		else if (!locationConfig->alias.empty())
			resolvedPath = locationConfig->alias;
	}
	if (request.getMethod() == "GET" || request.getMethod() == "DELETE" || (request.getMethod() == "POST" && !locationConfig->cgi.empty()))
	{
		if (locationConfig != NULL && !locationConfig->alias.empty())  // ← Add NULL check
		{
			std::string remaining = request.getPath().substr(locationConfig->path.size());
			if (!remaining.empty() && remaining[0] == '/')
				remaining = remaining.substr(1);
			resolvedPath += remaining;
		}
		else
			resolvedPath += request.getPath();
	}
}
