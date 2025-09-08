#ifndef TOKEN_HPP
# define TOKEN_HPP

#include "./Header.hpp"

enum	TokenType
{
	Key,        //0
	Argument,   //1
	BlockStart, //2
	BlockEnd,   //3
	SimpleEnd   //4
};

class Token
{
	public:
		std::string	buffer;
		TokenType	type;
		int	line;

		Token(const std::string& str, TokenType typ, int level);
		~Token();
};

class Lexer
{
	private:
		std::ifstream	&_inputStream;
		std::string		_buffer;
		std::string::iterator	_start;
		std::string::iterator	_pos;
		int	_line;
	public:
		Lexer(std::ifstream	&inputStream);
		~Lexer();

		bool	isComment(void);
		bool	isDelimiter(void);
		void	tokenAppend(std::vector<Token>& tokens);
		void	tokenise(std::vector<Token>&	tokens);
};

#endif
