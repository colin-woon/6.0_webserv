#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <HttpRequest.hpp>

// ===== Config Parser =====
class Config
{
public:
	int port;
	std::string host;
	std::map<std::string, std::string> cgiMap;

	Config(const std::string &path)
	{
		std::ifstream file(path.c_str());
		std::string key, val1, val2;
		while (file >> key)
		{
			if (key == "port")
				file >> port;
			else if (key == "host")
				file >> host;
			else if (key == "cgi_extension")
			{
				file >> val1 >> val2;
				cgiMap[val1] = val2;
			}
		}
	}
};

// ===== Networking (raw socket + runner) =====
class ServerSocket
{
	int server_fd;
	struct sockaddr_in address;

public:
	void start(int port)
	{
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0)
		{
			std::cerr << "Socket creation failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		// Add SO_REUSEADDR option
		int opt = 1;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			std::cerr << "setsockopt failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		std::memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		{
			std::cerr << "Bind failed: Port " << port << " may already be in use" << std::endl;
			exit(EXIT_FAILURE);
		}

		if (listen(server_fd, 1) < 0)
		{
			std::cerr << "Listen failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		std::cout << "Listening on http://localhost:" << port << std::endl;
	}

	int acceptClient()
	{
		socklen_t addrlen = sizeof(address);
		return accept(server_fd, (struct sockaddr *)&address, &addrlen);
	}

	std::string readRequest(int client_fd)
	{
		char buffer[1024];
		std::memset(buffer, 0, sizeof(buffer));
		read(client_fd, buffer, sizeof(buffer) - 1);
		return std::string(buffer);
	}

	void sendResponse(int client_fd, const std::string &resp)
	{
		send(client_fd, resp.c_str(), resp.size(), 0);
	}
};

// ===== HTTP + CGI =====
class HttpHandler
{
	Config &config;

public:
	HttpHandler(Config &cfg) : config(cfg) {}

	std::string runCgi(const std::string &path)
	{
		int pipefd[2];
		pipe(pipefd);

		pid_t pid = fork();
		if (pid == 0)
		{
			// Child
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[0]);
			close(pipefd[1]);

			// Pick interpreter from config
			std::string ext = path.substr(path.find_last_of('.'));
			std::string interp = config.cgiMap[ext];

			char *args[3];
			args[0] = const_cast<char *>(interp.c_str());
			args[1] = const_cast<char *>(path.c_str());
			args[2] = NULL;

			execve(args[0], args, NULL);
			_exit(1);
		}
		else
		{
			// Parent
			close(pipefd[1]);
			char buffer[1024];
			std::memset(buffer, 0, sizeof(buffer));
			read(pipefd[0], buffer, sizeof(buffer) - 1);
			close(pipefd[0]);
			return std::string(buffer);
		}
	}

	std::string handleRequest(const std::string &raw)
	{
		// For demo, always run hello.py
		std::string cgiOut = runCgi("./cgi/test.py");

		// Wrap into proper HTTP response
		std::ostringstream resp;
		resp << "HTTP/1.1 200 OK\r\n";
		resp << "Content-Length: " << cgiOut.size() << "\r\n";
		resp << "\r\n";
		resp << cgiOut;
		return resp.str();
	}
};

// ===== Main =====
int main()
{
	Config config("./config/test_config.conf");
	ServerSocket server;
	server.start(config.port);

	while (true)
	{ // Add this loop to handle multiple requests
		int client_fd = server.acceptClient();
		std::string req = server.readRequest(client_fd);

		std::cout << "Received new request:\n"
				  << req << std::endl;

		HttpRequest request;
		request.parseRawRequest(req);

		std::cout << std::endl;
		std::cout << request << std::endl;

		HttpHandler http(config);
		std::string resp = http.handleRequest(req);

		server.sendResponse(client_fd, resp);
		close(client_fd);

		std::cout << "\n--- Ready for next request ---\n"
				  << std::endl;
	}

	return 0;
}
