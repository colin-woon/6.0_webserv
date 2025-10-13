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
			locationConfig = &serverConfig.location[i];
	}
}

void Router::getResolvedPath(HttpRequest &request, const Server &serverConfig)
{
	if (locationConfig != NULL)
	{
		if (!locationConfig->root.empty())
			resolvedPath = locationConfig->root + request.getPath();
		else
			resolvedPath = serverConfig.root + request.getPath();
	}
	else
		resolvedPath = serverConfig.root + request.getPath();

	struct stat path_stat;
	stat(resolvedPath.c_str(), &path_stat);

	if (S_ISDIR(path_stat.st_mode))
	{
		std::string indexFile;
		if (locationConfig == NULL)
			indexFile = serverConfig.index;
		else
		{
			if (!locationConfig->index.empty())
				indexFile = locationConfig->index;
			else
				indexFile = serverConfig.index;
		}
		if (!resolvedPath.empty() && resolvedPath[resolvedPath.size() - 1] != '/')
			resolvedPath += '/';
		resolvedPath += indexFile;
	}
	else if (S_ISREG(path_stat.st_mode))
		return;
	else
		throw Http404NotFoundException();
}
