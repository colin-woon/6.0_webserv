#include "networking/Sockets/Sockets.hpp"
#include "networking/ServerLoop/ServerLoop.hpp"
#include "../models/parsing/Parsing.hpp"

// bool setNonBlocking(int fd){
// 	int flags = fcntl (fd, F_GETFL, 0);
// 	if (flags == -1)
// 		return (false);
// 	return (fcntl(fd, F_GETFL, flags | O_NONBLOCK) != -1);
// }

int main(int argc, char **argv)
{

	try
	{
		Parsing parse(argc, argv);
		parse.parseNodes();
		std::vector<int> listenerFdList;
		listenerFdList = setupListenerSockets(parse.nodes);
		mainServerLoop(listenerFdList, parse.nodes);
		std::cout << "done" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
}

// int main(){
// 	const int PORT = 8080;
//
// 	// creates tcp listening socket on port 8080
// 	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (listen_fd == -1){
// 		std::cerr << "socket error" << std::endl;
// 		return (1);
// 	}
//
// 	// //allows quick rebinding during dev to avoid address already in use
// 	// int yes = 1;
// 	// if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
// 	// 	std::cerr<<"setsockopt error" << std::endl;
// 	// }
// 	//
// 	// if (!setNonBlocking(listen_fd)){
// 	// 	std::cerr << ("Set non blocking flags error!") << std::endl;
// 	// 	return (1);
// 	// }
//
// 	struct sockaddr_in addr;
// 	std::memset(&addr, 0, sizeof(addr));
// 	addr.sin_family = AF_INET;
// 	addr.sin_addr.s_addr = htonl(INADDR_ANY);
// 	addr.sin_port = htons(PORT);
//
// 	if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1){
// 		std::cerr << "bind error!" << std::endl;
// 		return (1);
// 	}
//
// 	if (listen(listen_fd, 128) == -1){
// 		std::cerr << "listen error!" << std::endl;
// 		return (1);
// 	}
// 	std::cout << "Currently listening on http://localhost:" << PORT << std::endl;
//
// 	// int epollFD = epoll_create(1);
// 	// if (epollFD == -1){
// 	// 	std::cerr << "Epoll create failed!" << std::endl;
// 	// 	return (1);
// 	// }
// 	//
//
// 	sockaddr_in client;
// 	socklen_t	client_len = sizeof(client);
// 	int client_fd = accept(listen_fd, (sockaddr*)&client, &client_len);
// 	if (client_fd == -1){
// 		std::cerr << "accept error!" << std::endl;
// 		return (1);
// 	}
// 	std::cout << "client has connected!" << std::endl;
// 		char buf[1024];
// 		memset(buf, 0, sizeof(buf));
// 		ssize_t bytes_received = read(client_fd, &buf, sizeof(buf));
// 		if (bytes_received == -1){
// 			std::cerr << "read from client_fd error." << std::endl;
// 			return (1);
// 		}
// 		else if (bytes_received == 0){
// 			std::cout << "client did not send anything..." << std::endl;
// 			// break;
// 		}
// 		else {
// 			std::cout << "webserv received " << bytes_received << " bytes!" << std::endl;
// 			write(client_fd, &buf, sizeof(buf));
// 		}
//
// 	close (client_fd);
// 	while (1){
// 		usleep(1);
// 	}
//
// 	close (listen_fd);
// 	return (0);
// }
