#include "./CGI.hpp"

CGI::CGI(const HttpRequest& request, const Server& srv) : _req(request)
{
	this->createEnv(srv);
}

CGI::~CGI() {}

CGI::CGISimpleException::CGISimpleException(const std::string message) : _message("CGI Error: " + message) {}
const char *CGI::CGISimpleException::what() const throw()
{
	return (this->_message.c_str());
}

std::string	CGI::execCGI(const Location& loc, const std::pair<std::string, std::string>& cgiEntry)
{
	int	sp[2];

	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sp) == -1)
		throw CGI::CGISimpleException("Socket Pair Failed");

	pid_t pid = fork();
	if (pid == -1)
	{
		close(sp[0]);
		close(sp[1]);
		throw CGI::CGISimpleException("Fork Failed");
	}
	if (!pid)
	{
		dup2(sp[1], STDIN_FILENO);
		dup2(sp[1], STDOUT_FILENO);
		close(sp[0]);
		close(sp[1]);

		char *argv[3];
		argv[0] = const_cast<char *>(cgiEntry.first.c_str());
		argv[1] = const_cast<char *>(cgiEntry.second.c_str());
		argv[2] = NULL;

		std::vector<char *>	envp;
		for (size_t i = 0; i < this->_envp.size(); i++)
			envp.push_back(const_cast<char *>(this->_envp[i].c_str()));
		envp.push_back(NULL);

		execve(argv[0], argv, envp.data());
		perror("CGI Execve Failed");
		exit (127);
	}
	else
	{
		close(sp[1]);
		struct pollfd	pfd;
		std::string	output;
		int	pollret;

		pfd.fd = sp[0];
		pfd.events = POLLIN | POLLOUT;
		bool	sendDone = _req.getBody().empty();
		bool	recvDone = false;

		size_t	sent;
		size_t	total = 0;
		char buf[1024];
		size_t	n;

		while (!sendDone || !recvDone)
		{
			pollret = poll(&pfd, 1, (loc.cgi_timeout_sec != -1) ? loc.cgi_timeout_sec * 1000 : 60000);
			if (pollret < 0)
			{
				perror("CGI Poll Failed");
				break ;
			}
			else if (!pollret)
			{
				perror("CGI Timeout");
				break ;
			}

			if ((pfd.revents & POLLOUT) && !sendDone)
			{
				sent = send(sp[0], _req.getBody().data() + total, _req.getBody().size() - total, 0);
				if (sent < 0)
				{
					perror("CGI Send Failed");
					sendDone = true;
				}
				else
				{
					total += sent;
					if (total < _req.getBody().size())
						sendDone = true;
				}
			}
			if ((pfd.revents & POLLIN) && !recvDone)
			{
				n = recv(sp[0], buf, sizeof(buf), 0);
				if (n <= 0)
				{
					recvDone = true;
					if (n < 0)
						perror("CGI recv Failed");
				}
				else
					output.append(buf, n);
			}
			if ((pfd.revents & POLLERR) || (pfd.revents & POLLHUP))
				recvDone = true;
		}
		close(sp[0]);
		return output;
	}
}

void	CGI::createEnv(const Server& srv)
{
	std::stringstream	stream;

	this->_envp.push_back("REQUEST_METHOD=" + this->_req.getMethod());
	this->_envp.push_back("SCRIPT_NAME=" + this->_req.getPath());

	std::string	query;
	for (int i = 0; i < this->_req.getQueryParams().size(); i++)
	{
		query.append(this->_req.getQueryParams().at(i).first);
		query.append("=");
		query.append(this->_req.getQueryParams().at(i).second);
		if (i < this->_req.getQueryParams().size() - 1)
			query.append("&");
	}
	this->_envp.push_back("QUERY_STRING=" + query);
	if (this->_req.getHeaders().count("Content-Length"))
		this->_envp.push_back("CONTENT_LENGTH=" + this->_req.getHeaders().find("Content-Length")->second);
	if (this->_req.getHeaders().count("Content-Type"))
		this->_envp.push_back("CONTENT_TYPE=" + this->_req.getHeaders().find("Content-Type")->second);

	this->_envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
	this->_envp.push_back("SERVER_PROTOCOL=" + this->_req.getVersion());
	this->_envp.push_back("SERVER_SOFTWARE=webserv/1.0");
	if (this->_req.getHeaders().count("host"))
		this->_envp.push_back("SERVER_NAME=" + this->_req.getHeaders().find("host")->second);
	else
		this->_envp.push_back("SERVER_NAME=" + srv.name[0]);
	stream << srv.port;
	this->_envp.push_back("SERVER_PORT=" + stream.str());
	this->_envp.push_back("REMOTE_ADDR=" + srv.host);

	std::string	key;
	for (std::map<std::string, std::string>::const_iterator it = this->_req.getHeaders().begin();
		it !=  this->_req.getHeaders().end(); it++)
	{
		if (it->first == "Content-Length" || it->first == "Content-Type")
			continue ;
		key = it->first;

		for (size_t i = 0; i < key.size(); i++)
			key[i] = (key[i] == '-') ? '_' : std::toupper(key[i]);
		this->_envp.push_back("HTTP_" + key + "=" + it->second);
	}
}

void CGI::handleCGI(HttpRequest &request, HttpResponse &response, const Server &serverConfig, Router &router)
{
			CGI cgi(request, serverConfig);

			const std::string &requestPath = request.getPath();
			size_t dotPos = requestPath.find_last_of(".");
			if (dotPos == std::string::npos)
				throw Http404NotFoundException();
			std::string cgiExtension = requestPath.substr(dotPos);
			std::string cgiInterpreter;
			try
			{
				cgiInterpreter = router.locationConfig->cgi.at(cgiExtension);
			}
			catch(const std::out_of_range &e)
			{
				throw Http502BadGatewayException();
			}
			const std::pair<std::string, std::string> cgiEntry = std::make_pair(router.resolvedPath, cgiInterpreter);

			std::string cgiOutput = cgi.execCGI(*router.locationConfig, cgiEntry);
			size_t separatorPos = cgiOutput.find("\r\n\r\n");

			std::string cgiBody;
			if (separatorPos != std::string::npos)
			{
				std::string cgiHeadersStr = cgiOutput.substr(0, separatorPos);
				cgiBody = cgiOutput.substr(separatorPos + 4);

				std::stringstream ss(cgiHeadersStr);
				std::string headerLine;
				while (std::getline(ss, headerLine) && !headerLine.empty() && headerLine != "\r")
				{
					size_t colonPos = headerLine.find(":");
					if (colonPos != std::string::npos)
					{
						std::string key = headerLine.substr(0, colonPos);
						std::string value = headerLine.substr(colonPos + 2); // Skip ": "
						// Trim potential trailing '\r' from value
						if (!value.empty() && value[value.size() - 1] == '\r') {
							value.erase(value.size() - 1);
						}
						response.addHeader(key, value);
					}
				}
			}
			else
			{
				// No headers from CGI, treat entire output as body
				cgiBody = cgiOutput;
			}

			response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK));
			response.setBody(cgiBody);

			// Set Content-Length based on the actual body size
			if (response.getHeaders().find("Content-Length") == response.getHeaders().end())
			{
				std::stringstream contentLength;
				contentLength << cgiBody.length();
				response.addHeader("Content-Length", contentLength.str());
			}
			return ;
}
