#include "../../includes/token.hpp"

token::token(const std::string& buffer, bool delimiter) : _buffer(buffer), _delimiter(delimiter) {}

token::~token() {}


lexer::lexer(std::string& input) : _start(input.end()), _pos(input.begin()), _config(input) {}

lexer::~lexer() {}

void	lexer::tokenAppend(std::deque<token>&	tokens)
{
	if (this->_start == this->_config.end())
	{
		std::string	buffer(1, *this->_pos);
		tokens.push_back(token(buffer, true));
	}
	else
	{
		std::string	buffer(this->_start, this->_pos);
		tokens.push_back(token(buffer, false));
	}
}

bool	lexer::isDelimiter(void)
{
	if (std::isspace(*this->_pos) || *this->_pos == ';' || *this->_pos == '}' || *this->_pos == '{')
		return (true);
	return (false);
}

void	lexer::tokenise(std::deque<token>&	tokens)
{
	while (this->_pos != this->_config.end())
	{
		if (!this->isDelimiter())
		{
			if (this->_start == this->_config.end())
				this->_start = this->_pos;
			this->_pos++;
			if (this->_pos == this->_config.end() || this->isDelimiter())
				tokenAppend(tokens);
			this->_pos--;
		}
		else if (!std::isspace(*this->_pos))
			tokenAppend(tokens);
		this->_pos++;
	}
}
