#include "../models/networking/Sockets/Sockets.hpp"
#include "../models/networking/ServerLoop/ServerLoop.hpp"
#include "../models/parsing/Parsing.hpp"

// bool setNonBlocking(int fd){
// 	int flags = fcntl (fd, F_GETFL, 0);
// 	if (flags == -1)
// 		return (false);
// 	return (fcntl(fd, F_GETFL, flags | O_NONBLOCK) != -1);
// }

// void printAllServers(const std::vector<Server>& servers) {
// 	for (size_t i = 0; i < servers.size(); ++i) {
// 		const Server& s = servers[i];
// 		std::cout << "\033[1;36m\n═══════════ Server #" << i + 1 << " ═══════════\033[0m\n";
// 		std::cout << "  \033[37mport:\033[0m \033[32m" << s.port << "\033[0m\n";
// 		std::cout << "  \033[37mhost:\033[0m \033[32m" << s.host << "\033[0m\n";
// 		std::cout << "  \033[37mroot:\033[0m \033[32m" << s.root << "\033[0m\n";
// 		std::cout << "  \033[37mindex:\033[0m \033[32m" << s.index << "\033[0m\n";
// 		std::cout << "  \033[37mautoindex:\033[0m " << (s.autoindex ? "\033[32mon\033[0m" : "\033[31moff\033[0m") << "\n";
// 		std::cout << "  \033[37mbacklog:\033[0m \033[32m" << s.backlog << "\033[0m\n";
// 		std::cout << "  \033[37mclient_timeout_sec:\033[0m \033[32m" << s.client_timeout_sec << "\033[0m\n";
// 		std::cout << "  \033[37mmax_body_size:\033[0m \033[32m" << s.max_body_size << "\033[0m\n";
// 		std::cout << "  \033[37mbody_ram_threshold:\033[0m \033[32m" << s.body_ram_threshold << "\033[0m\n";
// 		std::cout << "  \033[37mheader_cap:\033[0m \033[32m" << s.header_cap << "\033[0m\n";

// 		std::cout << "  \033[37mserver names:\033[0m ";
// 		for (size_t j = 0; j < s.name.size(); ++j)
// 			std::cout << "\033[32m" << s.name[j] << (j + 1 < s.name.size() ? ", " : "") << "\033[0m";
// 		std::cout << "\n";

// 		std::cout << "  \033[37merror_pages:\033[0m\n";
// 		for (std::map<int, std::string>::const_iterator it = s.error_pages.begin(); it != s.error_pages.end(); ++it)
// 			std::cout << "    [" << it->first << "] → \033[32m" << it->second << "\033[0m\n";

// 		for (size_t i = 0; i < s.location.size(); ++i) {
// 			const Location& loc = s.location[i];
// 			std::cout << "\033[1;33m\n    ── Location [" << loc.path << "] ──\033[0m\n";
// 			std::cout << "      \033[37mroot:\033[0m \033[32m" << loc.root << "\033[0m\n";
// 			std::cout << "      \033[37malias:\033[0m \033[32m" << loc.alias << "\033[0m\n";
// 			std::cout << "      \033[37mindex:\033[0m \033[32m" << loc.index << "\033[0m\n";
// 			std::cout << "      \033[37mupload_store:\033[0m \033[32m" << loc.upload_store << "\033[0m\n";
// 			std::cout << "      \033[37mredirect:\033[0m \033[32m" << loc.redirect.first << " → " << loc.redirect.second << "\033[0m\n";
// 			std::cout << "      \033[37mautoindex:\033[0m " << (loc.autoindex ? "\033[32mon\033[0m" : "\033[31moff\033[0m") << "\n";
// 			std::cout << "      \033[37mcgi_timeout_sec:\033[0m \033[32m" << loc.cgi_timeout_sec << "\033[0m\n";

// 			std::cout << "      \033[37mallowedMethods:\033[0m ";
// 			for (size_t k = 0; k < loc.allowedMethods.size(); ++k)
// 				std::cout << "\033[32m" << loc.allowedMethods[k] << (k + 1 < loc.allowedMethods.size() ? ", " : "") << "\033[0m";
// 			std::cout << "\n";

// 			std::cout << "      \033[37mCGI handlers:\033[0m\n";
// 			for (std::map<std::string, std::string>::const_iterator cgi = loc.cgi.begin(); cgi != loc.cgi.end(); ++cgi)
// 				std::cout << "        " << cgi->first << " → \033[32m" << cgi->second << "\033[0m\n";

// 			std::cout << "      \033[37mError pages:\033[0m\n";
// 			for (std::map<int, std::string>::const_iterator ep = loc.error_pages.begin(); ep != loc.error_pages.end(); ++ep)
// 				std::cout << "        [" << ep->first << "] → \033[32m" << ep->second << "\033[0m\n";
// 		}
// 		std::cout << "\n";
// 	}
// }

int main(int argc, char **argv)
{

	try
	{
		Parsing parse(argc, argv);
		parse.parseNodes();
		// printAllServers(parse.nodes);
		std::vector<int> listenerFdList;
		listenerFdList = setupListenerSockets(parse.nodes);
		ServerLoop headacheServer(listenerFdList, parse.nodes);
		headacheServer.run();
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
