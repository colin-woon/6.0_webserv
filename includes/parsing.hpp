#ifndef PARSING_HPP
# define PARSING_HPP

#include "./header.hpp"
#include "./token.hpp"
#include "./server.hpp"

class parsing
{
	public:
		parsing(std::ifstream& inputStream);
		~parsing();

		std::deque<token>	tokens;
		std::deque<server>	nodes;
};

#endif
