#include "../../includes/Parsing.hpp"

Parsing::Parsing(int argc, char **argv)
{
	if (argc != 2)
		throw Parsing::ParsingSimpleException("Invalid Argument Count");

	std::ifstream	inputStream(argv[1]);
	if (!inputStream.is_open())
		throw Parsing::ParsingSimpleException("Invalid File Path");

	Lexer	lex(inputStream);
	lex.tokenise(this->tokens);
	if (this->tokens.empty())
		throw Parsing::ParsingSimpleException("Empty Config");
}

Parsing::~Parsing() {}

Parsing::ParsingSimpleException::ParsingSimpleException(const std::string message) : _message("Parsing Error: " + message) {}
const char	*Parsing::ParsingSimpleException::what() const throw()
{ return (this->_message.c_str()); }

Parsing::ParsingSyntaxException::ParsingSyntaxException(Token &token, const std::string message)
{
	std::stringstream stream;
	stream << token.line;
	_message = "Parsing Error at line " + stream.str() + ": " + message;
}
const char	*Parsing::ParsingSyntaxException::what() const throw()
{ return (this->_message.c_str()); }

Parsing::ParsingInvalidContextException::ParsingInvalidContextException(Token &token, const std::string message)
{
	std::stringstream stream;
	stream << token.line;
	_message = "Parsing Error at line " + stream.str() + ": " + message + " \"" + token.buffer + "\"";
}
const char	*Parsing::ParsingInvalidContextException::what() const throw()
{ return (this->_message.c_str()); }

void	Parsing::toggleContext(int &level)
{
	if (this->_Context.second->type == BlockStart)
		level += 1;
	else if (this->_Context.second->type == BlockEnd)
		level -= 1;
}

void	Parsing::setContext(void)
{
	int level = 0;

	if (this->_Context.first == tokens.end() || this->_Context.first->type != BlockStart)
		throw Parsing::ParsingSyntaxException(*this->_Context.first, "Missing Server Context");
	_Context.second = this->_Context.first++;
	while (_Context.second != this->tokens.end())
	{
		toggleContext(level);
		if (!level)
			break ;
		_Context.second++;
	}
	if (_Context.second == this->tokens.end() || _Context.second->type != BlockEnd)
		throw Parsing::ParsingSyntaxException(*this->_Context.first, "Unclosed Context");
}

void	Parsing::parseNodes(void)
{
	_Context.first = this->tokens.begin();
	_Context.second = this->tokens.begin();
	while (_Context.first != this->tokens.end())
	{
		if (_Context.first->buffer == "server")
		{
			_Context.first++;
			setContext();
			this->nodes.push_back(Server(_Context));
		}
		else
			throw Parsing::ParsingInvalidContextException(*this->_Context.first, "Unknown Directive");
		_Context.second++;
		_Context.first = _Context.second;
	}
}
