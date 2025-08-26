#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "./header.hpp"

class location
{
	public:
		bool	autoindex;
		std::string	path;
		std::string	root;
		std::string	alias;
		std::string	index;
		std::string	cgi;
		std::string	upload_store;
		std::string	redirect;
		std::vector<std::string> methods;
		std::map<int, std::string>	error_pages;

		location();
		~location();

		class LocationSyntaxException : public std::exception
		{
			public:
				const char	*what() const throw();
		};
		class LocationInvalidContextException : public std::exception
		{
			public:
				const char	*what() const throw();
		};
};

#endif
