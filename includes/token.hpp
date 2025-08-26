#ifndef TOKEN_HPP
# define TOKEN_HPP

#include "./header.hpp"

class token
{
	public:
		std::string	buffer;
		bool	isDelimiter;
	
		token(const std::string& str, bool delimiter);
		~token();

};

class lexer
{
	private:
		std::string::iterator	_start;
		std::string::iterator	_pos;
		std::string		_config;
	public:
		lexer(std::string& input);
		~lexer();

		bool	isDelimiter(void);
		void	tokenAppend(std::deque<token>& tokens);
		void	tokenise(std::deque<token>&	tokens);
};

#endif
