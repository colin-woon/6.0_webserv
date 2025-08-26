#include "../../includes/parsing.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
		return (std::cerr << "Invalid input" << std::endl, 1);
	std::ifstream	inputStream(argv[1]);
	parsing parse(inputStream);
}
