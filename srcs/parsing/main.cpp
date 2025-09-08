#include "../../includes/Parsing.hpp"

int	main(int argc, char **argv)
{
	try
	{
		if (argc != 2)
			return (std::cerr << "Invalid input" << std::endl, 1);
		std::ifstream	inputStream(argv[1]);
		Parsing parse(inputStream);
		parse.parseNodes();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}
	return (0);
}

// for (size_t i = 0; i < parse.nodes.size(); i++)
// {
// 	std::cout << std::string(10, '=') << "SERVER" << std::string(10, '=') << std::endl;
// 	std::cout << parse.nodes[i].autoindex << std::endl;
// 	std::cout << parse.nodes[i].host << std::endl;
// 	for (size_t j = 0; j < parse.nodes[i].name.size(); j++)
// 		std::cout << parse.nodes[i].name[j] << std::endl;
// 	std::cout << parse.nodes[i].root << std::endl;
// 	std::cout << parse.nodes[i].index << std::endl;
// 	std::cout << parse.nodes[i].error_pages[404] << std::endl;
// 	std::cout << parse.nodes[i].port << std::endl;
// 	for (size_t z = 0; z < parse.nodes[i].location.size(); z++)
// 	{
// 		std::cout << std::string(10, '=') << "Location" << std::string(10, '=') << std::endl;
// 		std::cout << parse.nodes[i].location[z].index << std::endl;
// 		std::cout << parse.nodes[i].location[z].redirect.first << " " << parse.nodes[i].location[z].redirect.second << std::endl;
// 	}
// }
