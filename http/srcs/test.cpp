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
		std::memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		bind(server_fd, (struct sockaddr *)&address, sizeof(address));
		listen(server_fd, 1);
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
	Config config("./config/test_config.conf"); // Teammate A
	ServerSocket server;						// Teammate B
	server.start(config.port);

	int client_fd = server.acceptClient();
	std::string req = server.readRequest(client_fd);

	HttpHandler http(config); // Teammate C
	std::string resp = http.handleRequest(req);

	server.sendResponse(client_fd, resp);
	close(client_fd);
	return 0;
}
