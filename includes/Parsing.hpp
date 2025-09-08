#ifndef PARSING_HPP
# define PARSING_HPP

#include "./Header.hpp"
#include "./Token.hpp"
#include "./Server.hpp"

class Parsing
{
	private:
		std::pair<std::vector<Token>::iterator, std::vector<Token>::iterator>	_Context;
	public:
		Parsing(std::ifstream& inputStream);
		~Parsing();

		std::vector<Token>	tokens;
		std::vector<Server>	nodes;
		
		class EmptyConfigException : public std::exception
		{
			public:
				const char	*what() const throw();
		};
		class ParsingSyntaxException : public std::exception
		{
			private:
				std::string	_message;
			public:
				ParsingSyntaxException(Token &token, const std::string message);
				~ParsingSyntaxException() throw() {}
				const char	*what() const throw();
		};
		class ParsingInvalidContextException : public std::exception
		{
			private:
				std::string	_message;
			public:
				ParsingInvalidContextException(Token &token, const std::string message);
				~ParsingInvalidContextException() throw() {}
				const char	*what() const throw();
		};
		void	toggleContext(int &level);
		void	setContext(void);
		void	parseNodes(void);
};

#endif
