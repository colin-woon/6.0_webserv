#ifndef SERVER_HPP
# define SERVER_HPP

#include "./parsing.hpp"
#include "./location.hpp"

class server
{
	public:
		int	_port;
		int	_backlog;
		bool	_autoindex;
		std::string	_host;
		std::string _root;
		std::string _index;
		std::vector<std::string>	_name;
		std::map<int, std::string>	_error_pages;
		std::deque<location>	_chunks;

		int	_client_timeout_sec;
		size_t	_max_body_size;
		size_t	_body_ram_threshold;
		size_t	_header_cap;

		server();
		~server();

		class ServerSyntaxException : public std::exception
		{
			public:
				const char	*what() const throw();
		};
		class ServerInvalidContextException : public std::exception
		{
			public:
				const char	*what() const throw();
		};
};

#endif
