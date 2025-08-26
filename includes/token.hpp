#ifndef TOKEN_HPP
# define TOKEN_HPP

#include "./parsing.hpp"

class token
{
	public:
		bool	_delimiter;
		std::string	_buffer;
	
		token(const std::string& buffer, bool delimiter);
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
