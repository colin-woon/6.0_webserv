#ifndef CGI_HPP
# define CGI_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../http/Router.hpp"
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
		std::string	execCGI(const Location& loc, const std::pair<std::string, std::string>& cgiEntry);
		static void	handleCGI(HttpRequest &request, HttpResponse &response, const Server &serverConfig, Router &router);
};

#endif
