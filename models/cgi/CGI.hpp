#ifndef CGI_HPP
# define CGI_HPP

#include "../http/HttpRequest.hpp"
#include "../networking/ServerLoop/ServerLoop.hpp"
#include "../parsing/Server.hpp"

class CGI
{
	private:
		std::vector<std::string>	_envp;
		const HttpRequest& _req;
	public:
		CGI(const HttpRequest& request, const Server& srv);
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
		void	execCGI(ServerLoop& srvLoop, const std::pair<std::string, std::string>& cgiEntry);
};

#endif
