#include "../../includes/Server.hpp"

std::map<std::string, Server::directiveHandler>	Server::directiveMap;

void	stoiServer(Token& token, const std::string& str, int& value)
{
	char	*end = NULL;
	errno = 0;
	long val = std::strtol(str.c_str(), &end, 10);

	if (value != -1)
		throw Server::ServerInvalidContextException(token, "Duplicate Directive");
	if (errno == ERANGE || val > std::numeric_limits<int>::max() || val < std::numeric_limits<int>::min()
		|| *end != '\0')
		throw Server::ServerInvalidContextException(token, "Invalid Number");
	value = static_cast<int>(val);
	if (value < 0)
		throw Server::ServerInvalidContextException(token, "Improper Negative Number");
}

void	Server::handlePort(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	stoiServer(*srv._Context.first, srv._Context.first->buffer, srv.port);
	if (srv.port > 65535)
		throw Server::ServerInvalidContextException(*srv._Context.first, "Invalid Port");
}

void	Server::handleBacklog(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	stoiServer(*srv._Context.first, srv._Context.first->buffer, srv.backlog);
	if (srv.backlog < 1 || srv.backlog > SOMAXCONN)
		throw Server::ServerInvalidContextException(*srv._Context.first, "Invalid Backlog");
}

void	Server::handleClient_timeout_sec(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	stoiServer(*srv._Context.first, srv._Context.first->buffer, srv.client_timeout_sec);
	if (srv.client_timeout_sec > 86400)
		throw Server::ServerInvalidContextException(*srv._Context.first, "Timeout too large");
}

void	Server::handleAutoindex(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	if (srv.autoindex)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Duplicate Directive");
	if (srv._Context.first->buffer == "on")
		srv.autoindex = true;
	else if (srv._Context.first->buffer == "off")
		srv.autoindex = false;
	else
		throw Server::ServerInvalidContextException(*srv._Context.first, "Invalid Autoindex Setting");
}

void	Server::handleMax_body_size(Server& srv)
{
	int	suffix = 1;
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	char	last = srv._Context.first->buffer[srv._Context.first->buffer.size() - 1];
	if (std::toupper(last) == 'K')
		suffix = 1000;
	else if (std::toupper(last) == 'M')
		suffix = 1000000;
	if (suffix != 1)
		srv._Context.first->buffer.resize(srv._Context.first->buffer.size() - 1);	
	stoiServer(*srv._Context.first, srv._Context.first->buffer, srv.max_body_size);
	srv.max_body_size *= suffix;
}

void	Server::handleBody_ram_threshold(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	stoiServer(*srv._Context.first, srv._Context.first->buffer, srv.body_ram_threshold);
	if (srv.max_body_size != -1 && srv.body_ram_threshold > srv.max_body_size)
		throw Server::ServerInvalidContextException(*srv._Context.first, "Client_body_ram_threshold cannot be larger than Client_max_body_size");
}

void	Server::handleHeader_cap(Server& srv)
{
	int	suffix = 1;
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	char	last = srv._Context.first->buffer[srv._Context.first->buffer.size() - 1];
	if (std::toupper(last) == 'K')
		suffix = 1000;
	else if (std::toupper(last) == 'M')
		suffix = 1000000;
	if (suffix != 1)
		srv._Context.first->buffer.resize(srv._Context.first->buffer.size() - 1);	
	stoiServer(*srv._Context.first, srv._Context.first->buffer, srv.header_cap);
	srv.header_cap *= suffix;
}

void	Server::handleHost(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	srv.host = srv._Context.first->buffer;

	std::vector<std::string>	ipSplit;
	std::stringstream	ss(srv.host);
	std::string	substr;
	int	octet;

	while (ss.good())
	{
		std::getline(ss, substr, '.');
		ipSplit.push_back(substr);
	}
	if (ipSplit.size() != 4)
		throw Server::ServerInvalidContextException(*srv._Context.first, "Invalid Octet Number");
	for (size_t i = 0; i < ipSplit.size(); i++)
	{
		if (ipSplit[i].size() > 1 && ipSplit[i][0] == '0')
			throw Server::ServerInvalidContextException(*srv._Context.first, "Invalid Octet Structure");
		octet = -1;
		stoiServer(*srv._Context.first, ipSplit[i], octet);
		if (octet > 255)
			throw Server::ServerInvalidContextException(*srv._Context.first, "Octet exceeds 255");
	}
}

void	Server::handleRoot(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	srv.root = srv._Context.first->buffer;
}

void	Server::handleIndex(Server& srv)
{
	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
	srv.index = srv._Context.first->buffer;
}
void	Server::handleName(Server& srv)
{
	if (!srv.name.empty())
		throw Server::ServerInvalidContextException(*srv._Context.first, "Duplicate Directive");
	while (1)
	{
		srv._Context.first++;
		if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
			break ;
		srv.name.push_back(srv._Context.first->buffer);
	}
	srv._Context.first--;
	if (srv.name.empty())
		throw Server::ServerInvalidContextException(*(srv._Context.first - 1), "Missing Argument");
}

void	Server::handleError_page(Server& srv)
{
	std::vector<Token>::iterator	it;

	it = ++srv._Context.first;
	while (it != srv._Context.second && it->type == Argument)
		it++;
	if (it == srv._Context.second)
		throw Server::ServerInvalidContextException(*srv._Context.first, "Missing ';'");
	--it;
	while (srv._Context.first != it)
	{
		std::pair<int, std::string>	error;

		error.first = -1;
		stoiServer(*srv._Context.first, srv._Context.first->buffer, error.first);
		if (error.first < 400 || error.first > 599)
			throw Server::ServerInvalidContextException(*srv._Context.first, "Unrecognised Error Code");
		if (srv.error_pages.find(error.first) != srv.error_pages.end())
			throw Server::ServerInvalidContextException(*srv._Context.first, "Duplicate Directive");
		error.second = it->buffer;
		srv.error_pages.insert(error);
		srv._Context.first++;
	}
}

void	Server::handleLocation(Server& srv)
{
	std::vector<Token>::iterator	start;
	std::string	path;

	srv._Context.first++;
	if (srv._Context.first == srv._Context.second || srv._Context.first->type != Argument)
		throw Server::ServerSyntaxException(*srv._Context.first, "Missing Argument");
	path = srv._Context.first->buffer;
	start = ++srv._Context.first;
	if (start == srv._Context.second || start->type != BlockStart)
		throw Server::ServerSyntaxException(*srv._Context.first, "Missing Argument");
	start++;
	while (srv._Context.first != srv._Context.second)
	{
		if (srv._Context.first->type == BlockEnd)
			break ;
		srv._Context.first++;
	}
	if (srv._Context.first == srv._Context.second)
		throw Server::ServerSyntaxException(*srv._Context.first, "Unclosed Location Context");
	srv.location.push_back(Location(std::pair<std::vector<Token>::iterator,
			std::vector<Token>::iterator>(start, srv._Context.first), path));
	srv._Context.first--;
}

void	Server::fillDirectiveMap(void)
{
	if (!directiveMap.empty())
		return ;
	directiveMap["listen"] = &Server::handlePort;
	directiveMap["backlog"] = &Server::handleBacklog;
	directiveMap["autoindex"] = &Server::handleAutoindex;
	directiveMap["host"] = &Server::handleHost;
	directiveMap["index"] = &Server::handleIndex;
	directiveMap["server_name"] = &Server::handleName;
	directiveMap["root"] = &Server::handleRoot;
	directiveMap["body_ram_threshold"] = &Server::handleBody_ram_threshold;
	directiveMap["client_timeout_sec"] = &Server::handleClient_timeout_sec;
	directiveMap["client_max_body_size"] = &Server::handleMax_body_size;
	directiveMap["client_header_buffer_size"] = &Server::handleHeader_cap;
	directiveMap["error_page"] = &Server::handleError_page;
	directiveMap["location"] = &Server::handleLocation;
}

Server::Server(std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator> context)
	: _Context(context), port(-1), backlog(-1), client_timeout_sec(-1),
		max_body_size(-1), body_ram_threshold(-1), header_cap(-1), autoindex(false)
{
	fillDirectiveMap();
	parseDirectives();
}

Server::~Server() {}

Server::ServerSyntaxException::ServerSyntaxException(Token& token, const std::string message)
{
	std::stringstream stream;
	stream << token.line;
	_message = "Error at line " + stream.str() + ": " + message;
}
const char	*Server::ServerSyntaxException::what() const throw()
{
	return (this->_message.c_str());
}

Server::ServerInvalidContextException::ServerInvalidContextException(Token& token, const std::string message)
{
	std::stringstream stream;
	stream << token.line;
	_message = "Error at line " + stream.str() + ": " + message + " \"" + token.buffer + "\"";
}
const char	*Server::ServerInvalidContextException::what() const throw()
{ return (this->_message.c_str()); }

void	Server::parseDirectives(void)
{
	std::map<std::string, directiveHandler>::iterator	it;
	while (this->_Context.first != this->_Context.second)
	{
		if (this->_Context.first->type == Key)
		{
			it = directiveMap.find(this->_Context.first->buffer);
			if (it == directiveMap.end())
				throw Server::ServerInvalidContextException(*this->_Context.first, "Unknown Directive");
			else
				it->second(*this);

			if ((++this->_Context.first)->type != SimpleEnd && this->_Context.first->type != BlockEnd)
				throw Server::ServerSyntaxException(*this->_Context.first, "Missing ';'");
		}
		else
			throw Server::ServerSyntaxException(*this->_Context.first, "Missing Key");
		this->_Context.first++;
	}
}
