#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "./Header.hpp"
#include "./Token.hpp"

class Location
{
	private:
		std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator>	_Context;
		
		typedef void	(*directiveHandler)(Location&);
		static std::map<std::string, directiveHandler>	directiveMap;

		static void	handleAutoindex(Location& loc);
		static void	handlePath(Location& loc);
		static void	handleRoot(Location& loc);
		static void	handleAlias(Location& loc);
		static void	handleIndex(Location& loc);
		static void	handleCgi(Location& loc);
		static void	handleUpload_store(Location& loc);
		static void	handleRedirect(Location& loc);
		static void	handleAllowedMethods(Location& loc);
		static void	handleError_page(Location& loc);
		static void	fillDirectiveMap(void);
	public:
		bool	autoindex;
		std::string	path;
		std::string	root;
		std::string	alias;
		std::string	index;
		std::string	upload_store;
		std::pair<int, std::string>	redirect;
		std::map<std::string, std::string>	cgi;
		std::vector<std::string> allowedMethods;
		std::map<int, std::string>	error_pages;

		Location(std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator> Context, const std::string& pth);
		~Location();

		class LocationSyntaxException : public std::exception
		{
			private:
				std::string _message;
			public:
				LocationSyntaxException(const std::string& error);
				~LocationSyntaxException() throw() {}
				const char	*what() const throw();
		};
		class LocationInvalidContextException : public std::exception
		{
			private:
				std::string	_message;
			public:
				LocationInvalidContextException(const std::string& error);
				~LocationInvalidContextException() throw() {}
				const char	*what() const throw();
		};
		void	parseDirectives(void);
};

#endif
