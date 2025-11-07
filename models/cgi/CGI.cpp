#include "./CGI.hpp"
#include "../networking/ServerLoop/ServerLoop.hpp"

CGI::CGI(Client& client, const Server &srv) : _client(client)
{
	this->createEnv(srv);
	// std::cout << "This is the ENVIRONMENT: " << std::endl;
	// for (size_t i = 0; i < this->_envp.size(); i++)
		// std::cout << this->_envp[i] << std::endl;
}

CGI::~CGI() {}

CGI::CGISimpleException::CGISimpleException(const std::string message) : _message("CGI Error: " + message) {}
const char *CGI::CGISimpleException::what() const throw()
{
	return (this->_message.c_str());
}

void	CGI::execCGI(ServerLoop& srvLoop, const std::pair<std::string, std::string>& cgiEntry, Router& router)
{
	int fdRead[2];
	int fdWrite[2];
	if (pipe(fdRead) < 0 || pipe(fdWrite) < 0)
	{
		std::cout << "CGI Pipe Fail" << std::endl;
		return ;
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		close(fdRead[0]);
		close(fdRead[1]);
		close(fdWrite[0]);
		close(fdWrite[1]);
		throw CGI::CGISimpleException("Fork Failed");
	}
	if (!pid)
	{
		dup2(fdRead[0], STDIN_FILENO);
		dup2(fdWrite[1], STDOUT_FILENO);
		close(fdRead[0]);
		close(fdRead[1]);
		close(fdWrite[0]);
		close(fdWrite[1]);

		char *argv[3];
		if (cgiEntry.first.empty())
		{
			argv[0] = const_cast<char *>(cgiEntry.second.c_str());
			argv[1] = NULL;
		}
		else
		{
			argv[0] = const_cast<char *>(cgiEntry.first.c_str());
			argv[1] = const_cast<char *>(cgiEntry.second.c_str());
		}
		argv[2] = NULL;

		std::vector<char *> envp;
		for (size_t i = 0; i < this->_envp.size(); i++)
			envp.push_back(const_cast<char *>(this->_envp[i].c_str()));
		envp.push_back(NULL);

		execve(argv[0], argv, envp.data());
		perror("CGI Execve Failed");
		exit(127);
	}
	else
	{
		close(fdRead[0]);
		close(fdWrite[1]);
		struct pollfd	pfd;

		fcntl(fdRead[1], F_SETFL, O_NONBLOCK);
		pfd.fd = fdRead[1];
		pfd.events = POLLOUT;
		pfd.revents = 0;
		srvLoop.addPollCGI(pfd, pid, _client, router, fdWrite[0]);

		fcntl(fdWrite[0], F_SETFL, O_NONBLOCK);
		pfd.fd = fdWrite[0];
		pfd.events = POLLIN;
		pfd.revents = 0;
		srvLoop.addPollCGI(pfd, pid, _client, router, fdRead[1]);
		return ;
	}
}

void CGI::createEnv(const Server &srv)
{
	std::stringstream stream;
	const HttpRequest& req = _client.request;

	this->_envp.push_back("REQUEST_METHOD=" + req.getMethod());
	this->_envp.push_back("SCRIPT_NAME=" + req.getPath());

	std::string query;
	for (size_t i = 0; i < req.getQueryParams().size(); i++)
	{
		query.append(req.getQueryParams().at(i).first);
		query.append("=");
		query.append(req.getQueryParams().at(i).second);
		if (i < req.getQueryParams().size() - 1)
			query.append("&");
	}
	this->_envp.push_back("QUERY_STRING=" + query);
	if (req.getHeaders().count("Content-Length"))
		this->_envp.push_back("CONTENT_LENGTH=" + req.getHeaders().find("Content-Length")->second);
	if (req.getHeaders().count("Content-Type"))
		this->_envp.push_back("CONTENT_TYPE=" + req.getHeaders().find("Content-Type")->second);

	this->_envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
	this->_envp.push_back("SERVER_PROTOCOL=" + req.getVersion());
	this->_envp.push_back("SERVER_SOFTWARE=webserv/1.0");
	if (req.getHeaders().count("Host"))
		this->_envp.push_back("SERVER_NAME=" + req.getHeaders().find("Host")->second);
	else
		this->_envp.push_back("SERVER_NAME=" + srv.name[0]);
	stream << srv.port;
	this->_envp.push_back("SERVER_PORT=" + stream.str());
	this->_envp.push_back("REMOTE_ADDR=" + srv.host);

	std::string key;
	for (std::map<std::string, std::string>::const_iterator it = req.getHeaders().begin();
		 it != req.getHeaders().end(); it++)
	{
		if (it->first == "Content-Length" || it->first == "Content-Type")
			continue;
		key = it->first;

		for (size_t i = 0; i < key.size(); i++)
			key[i] = (key[i] == '-') ? '_' : std::toupper(key[i]);
		this->_envp.push_back("HTTP_" + key + "=" + it->second);
	}
}

void CGI::handleCGI(Client& client, Router &router)
{
	const std::string &requestPath = client.request.getPath();
	size_t dotPos = requestPath.find_last_of(".");
	if (dotPos == std::string::npos)
		throw Http404NotFoundException();
	std::string cgiExtension = requestPath.substr(dotPos);
	std::string cgiInterpreter;
	try
	{
		if (cgiExtension != ".cgi")
			cgiInterpreter = router.locationConfig->cgi.at(cgiExtension);
	}
	catch (const std::out_of_range &e)
	{
		throw Http502BadGatewayException();
	}
	const std::pair<std::string, std::string> cgiEntry = std::make_pair(cgiInterpreter, router.resolvedPath);
	client.response.isCGI = true;
	this->execCGI(*client.srvLoop_, cgiEntry, router);
	return;
}

void CGI::getHttpResponse(CGIcontext& ctx)
{
	std::string seperator = "\n\n";
	size_t separatorPos = ctx.buffer.find(seperator);
	std::string cgiBody;
	if (separatorPos != std::string::npos)
	{
		std::string cgiHeadersStr = ctx.buffer.substr(0, separatorPos);
		cgiBody = ctx.buffer.substr(separatorPos + seperator.size());

		std::stringstream ss(cgiHeadersStr);
		std::string headerLine;
		while (std::getline(ss, headerLine) && !headerLine.empty() && headerLine != "\r")
		{
			size_t colonPos = headerLine.find(":");
			if (colonPos != std::string::npos)
			{
				std::string key = headerLine.substr(0, colonPos);
				std::string value = headerLine.substr(colonPos + 2);
				if (!value.empty() && value[value.size() - 1] == '\r')
					value.erase(value.size() - 1);
				if (key == "Status")
					ctx.response.setStatusCode(value);
				else
					ctx.response.addHeader(key, value);
			}
		}
	}
	else
		cgiBody = ctx.buffer;

	if (ctx.response.getStatusCode().empty())
		ctx.response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK));

	// Check if CGI returned an error status code
	std::string statusCode = ctx.response.getStatusCode();
	if (statusCode[0] == '4' || statusCode[0] == '5')
	{
		// Create exception and generate error page
		HttpException* exception = HttpException::createFromStatusCode(statusCode);
		if (exception && ctx.serverConfig)
		{
			handleHttpException(*ctx.serverConfig, ctx.loc, *exception, ctx.response);
			delete exception;
			return;
		}
		delete exception;
	}

	ctx.response.setBody(cgiBody);
	if (ctx.response.getHeaders().find("Content-Length") == ctx.response.getHeaders().end())
	{
		std::stringstream contentLength;
		contentLength << cgiBody.length();
		ctx.response.addHeader("Content-Length", contentLength.str());
	}
}
