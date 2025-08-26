#include "./parsing.hpp"

parsing::parsing(std::ifstream& inputStream)
{
	std::string	config;
	std::string	line;

	while (std::getline(inputStream, line))
		config += line;

	lexer	lex(config);
}

parsing::~parsing() {}
