#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "./Header.hpp"
#include "./Token.hpp"

class Spec
{
	public:
		std::string path;
		std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator> Context;

		Spec(const std::string& pth, std::vector<Token>::iterator Start, std::vector<Token>::iterator End)
			: path(pth), Context({Start, End}) {}
		~Spec() {}

		bool	operator==(const std::string path) { return (path == this->path); }
		bool	operator>(const std::string path) { return (path.size() > this->path.size()); }
};

class Location
{
	private:
		std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator>	_Context;
		std::string	_srvRoot;
		std::map<int, std::string> _srvError_pages;

		typedef void	(*_directiveHandler)(Location&);
		static std::map<std::string, _directiveHandler>	_directiveMap;
		static std::vector<std::string>	_totalMethods;

		static void	handleAutoindex(Location& loc);
		static void	handleCGI_timeout_sec(Location& loc);
		static void	handleRoot(Location& loc);
		static void	handleAlias(Location& loc);
		static void	handleIndex(Location& loc);
		static void	handleCgi(Location& loc);
		static void	handleUpload_store(Location& loc);
		static void	handleRedirect(Location& loc);
		static void	handleAllowedMethods(Location& loc);
		static void	handleError_page(Location& loc);
		static void	fillDirectiveMap(void);

		void	stoiLocation(const std::string& str, int& value);
	public:
		bool	autoindex;
		int	cgi_timeout_sec;
		std::string path;
		std::string	root;
		std::string	alias;
		std::string	index;
		std::string	upload_store;
		std::pair<int, std::string>	redirect;
		std::map<std::string, std::string>	cgi;
		std::vector<std::string> allowedMethods;
		std::map<int, std::string>	error_pages;

		Location(const Spec& spec, const std::string& srvRoot, const std::map<int, std::string>& srvError_pages);
		~Location();
		
		class LocationSimpleException : public std::exception
		{
			private:
				std::string _message;
			public:
				LocationSimpleException(const std::string message);
				~LocationSimpleException() throw() {}
				const char	*what() const throw();
		};
		class LocationSyntaxException : public std::exception
		{
			private:
				std::string _message;
			public:
				LocationSyntaxException(Token& token, const std::string message);
				~LocationSyntaxException() throw() {}
				const char	*what() const throw();
		};
		class LocationInvalidContextException : public std::exception
		{
			private:
				std::string	_message;
			public:
				LocationInvalidContextException(Token& token, const std::string message);
				~LocationInvalidContextException() throw() {}
				const char	*what() const throw();
		};
		void	checkDirectives(void);
		void	parseDirectives(void);
};

#endif
