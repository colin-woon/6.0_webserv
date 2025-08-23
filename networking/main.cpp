#include "Network.hpp"

// bool setNonBlocking(int fd){
// 	int flags = fcntl (fd, F_GETFL, 0);
// 	if (flags == -1)
// 		return (false);
// 	return (fcntl(fd, F_GETFL, flags | O_NONBLOCK) != -1);
// }

int main(){
	const int PORT = 8080;

	// creates tcp listening socket on port 8080
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1){
		std::cerr << "socket error" << std::endl;
		return (1);
	}
	
	// //allows quick rebinding during dev to avoid address already in use
	// int yes = 1;
	// if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
	// 	std::cerr<<"setsockopt error" << std::endl;
	// }
	//
	// if (!setNonBlocking(listen_fd)){
	// 	std::cerr << ("Set non blocking flags error!") << std::endl;
	// 	return (1);
	// }

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

	if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1){
		std::cerr << "bind error!" << std::endl;
		return (1);
	}

	if (listen(listen_fd, 128) == -1){
		std::cerr << "listen error!" << std::endl;
		return (1);
	}
	std::cout << "Currently listening on http://localhost:" << PORT << std::endl;

	// int epollFD = epoll_create(1);
	// if (epollFD == -1){
	// 	std::cerr << "Epoll create failed!" << std::endl;
	// 	return (1);
	// }
	//
	
	while (true)
		sleep(1);

	close (listen_fd);
	return (0);
}
