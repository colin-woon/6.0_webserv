#ifndef SERVER_HPP
# define SERVER_HPP

#include "./header.hpp"
#include "./location.hpp"

class server
{
	public:
		int	port;
		int	backlog;
		bool	autoindex;
		std::string	host;
		std::string root;
		std::string index;
		std::vector<std::string>	name;
		std::map<int, std::string>	error_pages;
		std::deque<location>	chunks;

		int	client_timeout_sec;
		size_t	max_body_size;
		size_t	body_ram_threshold;
		size_t	header_cap;

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
