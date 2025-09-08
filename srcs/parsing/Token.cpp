#include "../../includes/Token.hpp"

Token::Token(const std::string& str, TokenType typ, int level) : buffer(str), type(typ), line(level) {}

Token::~Token() {}

Lexer::Lexer(std::ifstream &inputStream) :  _inputStream(inputStream), _line(0) {}

Lexer::~Lexer() {}

void	Lexer::tokenAppend(std::vector<Token>&	tokens)
{
	TokenType	tpe;

	if (this->_start == this->_buffer.end())
	{
		std::string	buffer(1, *this->_pos);
		tpe = SimpleEnd;
		if (*this->_pos == '{')
			tpe = BlockStart;
		else if (*this->_pos == '}')
			tpe = BlockEnd;
		tokens.push_back(Token(buffer, tpe, this->_line));
	}
	else
	{
		std::string	buffer(this->_start, this->_pos);
		if (tokens.empty() || (tokens.back().type == BlockStart
				|| tokens.back().type == BlockEnd
				|| tokens.back().type == SimpleEnd))
			tpe = Key;
		else
			tpe = Argument;
		tokens.push_back(Token(buffer, tpe, this->_line));
	}
	this->_start = this->_buffer.end();
}

bool	Lexer::isComment(void)
{
	if (*this->_pos == '#')
		return true;
	return false;
}

bool	Lexer::isDelimiter(void)
{
	if (*this->_pos == ';' || *this->_pos == '}' || *this->_pos == '{'
		|| std::isspace(*this->_pos) || this->isComment())
		return (true);
	return (false);
}

void	Lexer::tokenise(std::vector<Token>&	tokens)
{
	while (std::getline(this->_inputStream, this->_buffer))
	{
		this->_pos = this->_buffer.begin();
		this->_start = this->_buffer.end();
		this->_line++;
		while (this->_pos != this->_buffer.end())
		{
			if (!this->isDelimiter())
			{
				if (this->_start == this->_buffer.end())
					this->_start = this->_pos;
				this->_pos++;
				if (this->_pos == this->_buffer.end() || this->isDelimiter())
					tokenAppend(tokens);
				this->_pos--;
			}
			else if (!std::isspace(*this->_pos))
			{
				if (isComment())
					break ;
				tokenAppend(tokens);
			}
			this->_pos++;
		}
	}
}
