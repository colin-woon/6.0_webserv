#ifndef SERVER_HPP
# define SERVER_HPP

#include "./Header.hpp"
#include "./Location.hpp"
#include "./Token.hpp"

class Server
{
	private:
		std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator>	_Context;

		typedef void	(*_directiveHandler)(Server&);
		static std::map<std::string, _directiveHandler> _directiveMap;

		static void	handlePort(Server& srv);
		static void	handleBacklog(Server& srv);
		static void	handleClient_timeout_sec(Server& srv);
		static void	handleAutoindex(Server& srv);
		static void	handleMax_body_size(Server& srv);
		static void	handleBody_ram_threshold(Server& srv);
		static void	handleHeader_cap(Server& srv);
		static void	handleHost(Server& srv);
		static void	handleRoot(Server& srv);
		static void	handleIndex(Server& srv);
		static void	handleName(Server& srv);
		static void	handleError_page(Server& srv);
		static void	handleLocation(Server& srv);
		static void	fillDirectiveMap(void);

		void	stoiServer(const std::string& str, int& value);
	public:
		int	backlog;
		int	client_timeout_sec;
		int	max_body_size;
		int	body_ram_threshold;
		int	header_cap;
		bool	autoindex;
		std::string	host;
		std::string root;
		std::string index;
		std::vector<int>	port;
		std::vector<std::string>	name;
		std::map<int, std::string>	error_pages;
		std::vector<Location>	location;

		Server(std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator> context);
		~Server();

		class ServerSyntaxException : public std::exception
		{
			private:
				std::string _message;
			public:
				ServerSyntaxException(Token& token, const std::string message);
				~ServerSyntaxException() throw() {}
				const char	*what() const throw();
		};
		class ServerInvalidContextException : public std::exception
		{
			private:
				std::string	_message;
			public:
				ServerInvalidContextException(Token& token, const std::string message);
				~ServerInvalidContextException() throw() {}
				const char	*what() const throw();
		};
		void	parseDirectives(void);
};

#endif
