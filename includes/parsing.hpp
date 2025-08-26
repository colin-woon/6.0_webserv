#ifndef PARSING_HPP
# define PARSING_HPP

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <deque>
#include <map>
#include <exception>

#include "./token.hpp"
#include "./server.hpp"

class parsing
{
	public:
		parsing(std::ifstream& inputStream);
		~parsing();

		std::deque<token>	_tokens;
		std::deque<server>	_nodes;
};

#endif
