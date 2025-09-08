#include "../../includes/Location.hpp"

std::map<std::string, Location::directiveHandler>	Location::directiveMap;

void	stoiLocation(const std::string& str, int& value)
{
	char	*end = NULL;
	errno = 0;
	long val = std::strtol(str.c_str(), &end, 10);

	if (value != -1)
		throw Location::LocationInvalidContextException("Reassignment is not allowed");
	if (errno == ERANGE || val > std::numeric_limits<int>::max() || val < std::numeric_limits<int>::min()
		|| *end != '\0')
		throw Location::LocationInvalidContextException("Not a Proper Number");
	value = static_cast<int>(val);
	if (value < 0)
		throw Location::LocationInvalidContextException("Improper Negative Number");
}

void	Location::handleAutoindex(Location& loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc.autoindex)
		throw Location::LocationInvalidContextException("Reassignment is not allowed");
	if (loc._Context.first->buffer == "on")
		loc.autoindex = true;
	else if (loc._Context.first->buffer == "off")
		loc.autoindex = false;
	else
		throw Location::LocationInvalidContextException("Invalid Autoindex Setting");
}

void	Location::handlePath(Location& loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Path Required");
	loc.path = loc._Context.first->buffer;
}

void	Location::handleRoot(Location& loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Root Required");
	loc.root = loc._Context.first->buffer;
}

void	Location::handleAlias(Location& loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Alias Required");
	loc.alias = loc._Context.first->buffer;
}

void	Location::handleIndex(Location& loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Index Required");
	loc.index = loc._Context.first->buffer;
}

void	Location::handleUpload_store(Location& loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Upload Store Required");
	loc.upload_store = loc._Context.first->buffer;
}

void	Location::handleRedirect(Location& loc)
{
	if (loc.redirect.first != -1 || !loc.redirect.second.empty())
		throw Location::LocationInvalidContextException("Reasignment of redirection not allowed");
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Error code for redirect required");
	try
	{
		stoiLocation(loc._Context.first->buffer, loc.redirect.first);
	}
	catch(const std::exception& e)
	{
		loc._Context.first--;
		loc.redirect.first = 302;
	}
	loc._Context.first++;
	if (loc._Context.first != loc._Context.second && loc._Context.first->type == Argument)
		loc.redirect.second = loc._Context.first->buffer;
}

void	Location::handleCgi(Location& loc)
{
	std::pair<std::string, std::string>	entry;
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Extension for cgi required");
	entry.first = loc._Context.first->buffer;

	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException("Not enough arguments");
	if (loc._Context.first->buffer.empty())
		throw Location::LocationInvalidContextException("Path for cgi required");
	entry.second = loc._Context.first->buffer;
	loc.cgi.insert(entry);
}

void	Location::handleAllowedMethods(Location& loc)
{
	if (!loc.allowedMethods.empty())
		throw Location::LocationInvalidContextException("Reassignment of allowed methods is not allowed");
	std::vector<std::string>	methods;
	methods.push_back("GET");
	methods.push_back("POST");
	methods.push_back("DELETE");
	methods.push_back("PUT");
	methods.push_back("HEAD");
	while (1)
	{
		loc._Context.first++;
		if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
			break ;
		if (loc._Context.first->buffer.empty())
			throw Location::LocationInvalidContextException("Error code for redirect required");
		if (std::find(methods.begin(), methods.end(), loc._Context.first->buffer) == methods.end())
			throw Location::LocationInvalidContextException("Unknown HTTP method");
		loc.allowedMethods.push_back(loc._Context.first->buffer);
	}
	loc._Context.first--;
	if (loc.allowedMethods.empty())
		throw Location::LocationInvalidContextException("Not enough arguments");
}

void	Location::handleError_page(Location& loc)
{
	std::vector<Token>::iterator	it;

	it = ++loc._Context.first;
	while (it != loc._Context.second && it->type == Argument)
		it++;
	if (it == loc._Context.second)
		throw Location::LocationInvalidContextException("Missing ';' at the end of the directive");
	--it;
	while (loc._Context.first != it)
	{
		std::pair<int, std::string>	error;

		error.first = -1;
		if (loc._Context.first->buffer.empty())
			throw Location::LocationInvalidContextException("Error code required");
		stoiLocation(loc._Context.first->buffer, error.first);
		if (error.first < 400 || error.first > 599)
			throw Location::LocationInvalidContextException("Error code not recognised");
		if (loc.error_pages.find(error.first) != loc.error_pages.end())
			throw Location::LocationInvalidContextException("Reassignment of error codes is not allowed");
		error.second = it->buffer;
		loc.error_pages.insert(error);
		loc._Context.first++;
	}
}

void	Location::fillDirectiveMap(void)
{
	if (!directiveMap.empty())
		return ;
	directiveMap["autoindex"] = &Location::handleAutoindex;
	directiveMap["path"] = &Location::handlePath;
	directiveMap["root"] = &Location::handleRoot;
	directiveMap["alias"] = &Location::handleAlias;
	directiveMap["index"] = &Location::handleIndex;
	directiveMap["upload_store"] = &Location::handleUpload_store;
	directiveMap["return"] = &Location::handleRedirect;
	directiveMap["cgi"] = &Location::handleCgi;
	directiveMap["allow_methods"] = &Location::handleAllowedMethods;
	directiveMap["error_page"] = &Location::handleError_page;
}

Location::Location(std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator> Context, const std::string& pth)
	: _Context(Context), autoindex(false), path(pth), redirect(-1, "")
{
	fillDirectiveMap();
	parseDirectives();
}

Location::~Location() {}

Location::LocationSyntaxException::LocationSyntaxException(const std::string& error)
	: _message("Invalid Location Syntax: " + error) {}
const char	*Location::LocationSyntaxException::what() const throw()
{
	return this->_message.c_str();
}

Location::LocationInvalidContextException::LocationInvalidContextException(const std::string& error)
	: _message("Invalid Location directive: " + error) {}
const char	*Location::LocationInvalidContextException::what() const throw()
{
	return this->_message.c_str();
}

void	Location::parseDirectives(void)
{
	std::map<std::string, directiveHandler>::iterator	it;
	while (this->_Context.first != this->_Context.second)
	{
		if (this->_Context.first->type == Key)
		{
			it = directiveMap.find(this->_Context.first->buffer);
			if (it == directiveMap.end())
				throw Location::LocationInvalidContextException("Unknown Directive");
			else
				it->second(*this);
			
			if ((++_Context.first)->type != SimpleEnd && _Context.first->type != BlockEnd)
				throw Location::LocationSyntaxException("Missing ';' at end of directive");
		}
		else
			throw Location::LocationSyntaxException("Could Not Find Key");
		this->_Context.first++;
	}
}
