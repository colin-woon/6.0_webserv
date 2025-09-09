#ifndef SERVERLOOP_HPP
#define SERVERLOOP_HPP

#include "../../../includes/server.hpp"
#include "../Sockets/Sockets.hpp"
#include <ostream>
#include <stdexcept>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <climits> 


class Client{
	public:
		Client();
		~Client();
		int fd;
		std::string inBuff;
		std::string outBuff;
		bool closeFlag;
		const Server* owner;

		int timeoutMs;
		long long	expiresAtMs;
};

void mainServerLoop(std::vector<int>& listenerFdList, std::vector<Server>& serverList);

#endif

