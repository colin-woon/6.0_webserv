#include "Location.hpp"

std::map<std::string, Location::_directiveHandler> Location::_directiveMap;
std::vector<std::string> Location::_totalMethods;

void Location::stoiLocation(const std::string &str, int &value)
{
	char *end = NULL;
	errno = 0;
	long val = std::strtol(str.c_str(), &end, 10);

	if (value != -1)
		throw Location::LocationInvalidContextException(*(this->_Context.first - 1), "Duplicate Directive");
	if (errno == ERANGE || val > std::numeric_limits<int>::max() || val < std::numeric_limits<int>::min() || *end != '\0')
		throw Location::LocationInvalidContextException(*this->_Context.first, "Invalid Number");
	value = static_cast<int>(val);
	if (value < 0)
		throw Location::LocationInvalidContextException(*this->_Context.first, "Improper Negative Number");
}

void Location::handleAutoindex(Location &loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Missing Argument");
	if (loc.autoindex)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Duplicate Directive");
	if (loc._Context.first->buffer == "on")
		loc.autoindex = true;
	else if (loc._Context.first->buffer == "off")
		loc.autoindex = false;
	else
		throw Location::LocationInvalidContextException(*loc._Context.first, "Invalid Autoindex Setting");
}

void Location::handleCGI_timeout_sec(Location &loc)
{
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Missing Argument");
	loc.stoiLocation(loc._Context.first->buffer, loc.cgi_timeout_sec);
}

void Location::handleRoot(Location &loc)
{
	if (!loc.root.empty())
		throw Location::LocationInvalidContextException(*loc._Context.first, "Duplicate Directive");
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Missing Argument");
	if (!loc.alias.empty())
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Alias Predefined");
	loc.root = loc._Context.first->buffer;
	// if (loc.root[0] != '/')
	// 	loc.root = loc._srvRoot + "/" + loc.root;
}

void Location::handleAlias(Location &loc)
{
	if (!loc.alias.empty())
		throw Location::LocationInvalidContextException(*loc._Context.first, "Duplicate Directive");
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Missing Argument");
	if (!loc.root.empty())
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Root Predefined");
	loc.alias = loc._Context.first->buffer;
	// if (loc.alias[0] != '/')
	// 	loc.alias = loc._srvRoot + "/" + loc.alias;
}

void Location::handleIndex(Location &loc)
{
	if (!loc.index.empty())
		throw Location::LocationInvalidContextException(*loc._Context.first, "Duplicate Directive");
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Missing Argument");
	loc.index = loc._Context.first->buffer;
}

void Location::handleUpload_store(Location &loc)
{
	if (!loc.upload_store.empty())
		throw Location::LocationInvalidContextException(*loc._Context.first, "Duplicate Directive");
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Missing Argument");
	loc.upload_store = loc._Context.first->buffer;
	if (loc.upload_store[0] != '/')
		loc.upload_store = loc._srvRoot + "/" + loc.upload_store;
}

void Location::handleRedirect(Location &loc)
{
	if (loc.redirect.first != -1 || !loc.redirect.second.empty())
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Duplicate Directive");
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first - 1), "Missing Argument");
	try
	{
		loc.stoiLocation(loc._Context.first->buffer, loc.redirect.first);
	}
	catch (const std::exception &e)
	{
		loc._Context.first--;
		loc.redirect.first = 302;
	}
	loc._Context.first++;
	if (loc._Context.first != loc._Context.second && loc._Context.first->type == Argument)
		loc.redirect.second = loc._Context.first->buffer;
}

void Location::handleCgi(Location &loc)
{
	std::pair<std::string, std::string> entry;
	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first), "Missing Argument");
	entry.first = loc._Context.first->buffer;

	loc._Context.first++;
	if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
		throw Location::LocationInvalidContextException(*(loc._Context.first), "Missing Argument");
	entry.second = loc._Context.first->buffer;
	if (entry.second[0] != '/')
		entry.second = loc._srvRoot + "/" + entry.second;

	loc.cgi.insert(entry);
}

void Location::handleAllowedMethods(Location &loc)
{
	std::vector<Token>::iterator it = loc._Context.first;
	if (!loc.allowedMethods.empty())
		throw Location::LocationInvalidContextException(*loc._Context.first, "Duplicate Directive");
	while (1)
	{
		loc._Context.first++;
		if (loc._Context.first == loc._Context.second || loc._Context.first->type != Argument)
			break;
		if (std::find(_totalMethods.begin(), _totalMethods.end(), loc._Context.first->buffer) == _totalMethods.end())
			throw Location::LocationInvalidContextException(*loc._Context.first, "Unknown HTTP Method");
		loc.allowedMethods.push_back(loc._Context.first->buffer);
	}
	loc._Context.first--;
	if (loc.allowedMethods.empty())
		throw Location::LocationInvalidContextException(*it, "Missing Argument");
}

void Location::handleError_page(Location &loc)
{
	std::vector<Token>::iterator it;

	it = ++loc._Context.first;
	while (it != loc._Context.second && it->type == Argument)
		it++;
	if (it == loc._Context.second || it->type != SimpleEnd)
		throw Location::LocationInvalidContextException(*loc._Context.first, "Missing ';'");
	--it;
	while (loc._Context.first != it)
	{
		std::pair<int, std::string> error;

		error.first = -1;
		loc.stoiLocation(loc._Context.first->buffer, error.first);
		if (error.first < 400 || error.first > 599)
			throw Location::LocationInvalidContextException(*loc._Context.first, "Unrecognised Error Code");
		if (loc.error_pages.find(error.first) != loc.error_pages.end())
			throw Location::LocationInvalidContextException(*loc._Context.first, "Duplicate Directive");
		error.second = it->buffer;
		if (error.second[0] != '/')
			error.second = loc._srvRoot + "/" + error.second;
		loc.error_pages.insert(error);
		loc._Context.first++;
	}
}

void Location::fillDirectiveMap(void)
{
	if (!_directiveMap.empty())
		return;
	_directiveMap["autoindex"] = &Location::handleAutoindex;
	_directiveMap["cgi_timeout_sec"] = &Location::handleCGI_timeout_sec;
	_directiveMap["root"] = &Location::handleRoot;
	_directiveMap["alias"] = &Location::handleAlias;
	_directiveMap["index"] = &Location::handleIndex;
	_directiveMap["upload_store"] = &Location::handleUpload_store;
	_directiveMap["return"] = &Location::handleRedirect;
	_directiveMap["cgi"] = &Location::handleCgi;
	_directiveMap["allowed_methods"] = &Location::handleAllowedMethods;
	_directiveMap["error_page"] = &Location::handleError_page;
	_totalMethods.push_back("GET");
	_totalMethods.push_back("POST");
	_totalMethods.push_back("DELETE");
}

Location::Location(const Spec &spec, const std::string &srvRoot, const std::map<int, std::string> &srvError_pages)
	: _Context(spec.Context), _srvRoot(srvRoot), _srvError_pages(srvError_pages),
	  autoindex(false), cgi_timeout_sec(-1), path(spec.path), redirect(-1, "")
{
	fillDirectiveMap();
	parseDirectives();
}

Location::~Location() {}

Location::LocationSimpleException::LocationSimpleException(const std::string message)
{
	_message = "Location Error " + message;
}
const char *Location::LocationSimpleException::what() const throw()
{
	return (this->_message.c_str());
}

Location::LocationSyntaxException::LocationSyntaxException(Token &token, const std::string message)
{
	std::stringstream stream;
	stream << token.line;
	_message = "Location Error at line " + stream.str() + ": " + message;
}
const char *Location::LocationSyntaxException::what() const throw()
{
	return (this->_message.c_str());
}

Location::LocationInvalidContextException::LocationInvalidContextException(Token &token, const std::string message)
{
	std::stringstream stream;
	stream << token.line;
	_message = "Location Error at line " + stream.str() + ": " + message + " \"" + token.buffer + "\"";
}
const char *Location::LocationInvalidContextException::what() const throw()
{
	return (this->_message.c_str());
}

void Location::checkDirectives(void)
{
	if (this->cgi_timeout_sec == -1)
		this->cgi_timeout_sec = 60;
	if (this->root.empty() && this->alias.empty())
		this->root = this->_srvRoot;
	// if (this->index.empty())
	// 	this->index = "index.html";
	if (this->error_pages.empty())
		this->error_pages = this->_srvError_pages;
}

void Location::parseDirectives(void)
{
	std::map<std::string, _directiveHandler>::iterator it;
	while (this->_Context.first != this->_Context.second)
	{
		if (this->_Context.first->type == Key)
		{
			it = _directiveMap.find(this->_Context.first->buffer);
			if (it == _directiveMap.end())
				throw Location::LocationInvalidContextException(*this->_Context.first, "Unknown Directive");
			else
				it->second(*this);

			if ((++_Context.first)->type != SimpleEnd && _Context.first->type != BlockEnd)
				throw Location::LocationSyntaxException(*this->_Context.first, "Missing ';'");
		}
		else
			throw Location::LocationSyntaxException(*this->_Context.first, "Missing Key");
		this->_Context.first++;
	}
	checkDirectives();
}

const std::string &Location::getServerRoot() const
{
	return _srvRoot;
}
