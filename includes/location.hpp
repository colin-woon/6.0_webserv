#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "./parsing.hpp"

class location
{
	public:
		bool	_autoindex;
		std::string	_path;
		std::string	_root;
		std::string	_alias;
		std::string	_index;
		std::string	_cgi;
		std::string	_upload_store;
		std::string	_redirect;
		std::vector<std::string> _methods;
		std::map<int, std::string>	_error_pages;

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
