#ifndef SERVERLOOP_HPP
#define SERVERLOOP_HPP

#include "../../../includes/server.hpp"
#include "../Sockets/Sockets.hpp"


class Client {
	public:
		Client();
		~Client();
		int fd;
		std::string inBuff;
		std::string outBuff;
		bool closeFlag;
};

void mainServerLoop(std::vector<int>& listenerFdList, std::vector<Server>& serverList);

#endif

