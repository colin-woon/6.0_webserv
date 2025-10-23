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

void	CGI::execCGI(ServerLoop& srvLoop, const std::pair<std::string, std::string>& cgiEntry)
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
		fcntl(sp[0], F_SETFL, O_NONBLOCK);
		struct pollfd	pfd;
		pfd.fd = sp[0];
		pfd.events = POLLIN | POLLOUT;
		srvLoop.addSocket(pfd);
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
	if (this->_req.getHeaders().count("Host"))
		this->_envp.push_back("SERVER_NAME=" + this->_req.getHeaders().find("Host")->second);
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
