#ifndef CGI_HPP
# define CGI_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../parsing/Server.hpp"
#include "../http/Router.hpp"

class Client;
class ServerLoop;

struct CGIcontext
{
	pid_t pid;
	int fd;
	int clientFd;
	size_t sendPos;
	std::string buffer;
	int			timeoutMs;
	uint64_t	expiresAtMs;
	const Location	*loc;
	HttpResponse response;
};

class CGI
{
	private:
		std::vector<std::string>	_envp;
		Client& _client;
	public:
		CGI(Client& client, const Server& srv);
		~CGI();

		class CGISimpleException : public std::exception
		{
			private:
				std::string _message;
			public:
				CGISimpleException(const std::string message);
				~CGISimpleException() throw() {};
				const char	*what() const throw();
		};

		void	createEnv(const Server& srv);
		void	execCGI(ServerLoop& srvLoop, const std::pair<std::string, std::string>& cgiEntry, Router& router);
		void	handleCGI(Client& client, Router &router);
		static void	getHttpResponse(CGIcontext& ctx);
};

#endif
