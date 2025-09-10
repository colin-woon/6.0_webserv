#include "Sockets.hpp"
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>

static bool convStrToIpv4(std::string host, uint32_t &ipv4Net){
	if (host.empty() || host == "*" || host == "0.0.0.0"){
		ipv4Net = htonl(INADDR_ANY);
		return true;
	}
	if (host == "localhost"){
		ipv4Net = htonl(INADDR_LOOPBACK);
		return true;
	}
	in_addr temp_adr;
	if (inet_aton(host.c_str(), &temp_adr) != 0){
		ipv4Net = temp_adr.s_addr;
		return true;
	}
	return false;
}

static bool toSockAddr(std::string host, int port, sockaddr_in &socketAddr){
	std::memset(&socketAddr, 0, sizeof(socketAddr));
	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(port);

	uint32_t ipv4Net;
	if (convStrToIpv4(host, ipv4Net) == false)
		return false;
	socketAddr.sin_addr.s_addr = ipv4Net;
	return true;
}

bool setNonBlocking(int listenerFd){
	int flags = fcntl(listenerFd, F_GETFL, 0);
	if (flags == -1)
		return false;
	return (fcntl(listenerFd, F_SETFL, flags | O_NONBLOCK) != -1);
}

static bool openListeners(sockaddr_in socketAddr, int backlog, int &listenerFd){
	listenerFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenerFd == -1){
		std::cerr << "Socket function call error!" << std::endl;
		return false;
	}

	int yes = 1;
	if (setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
		std::cerr << "Setsockopt function call error!" << std::endl;
		return false;
	}

	if (setNonBlocking(listenerFd) == false){
		std::cerr << "Error when setting non-blocking on listener" << std::endl;
		return false;
	}

	if (bind(listenerFd, (sockaddr*)&socketAddr, sizeof(socketAddr)) == -1){
		std::cerr << "Socket binding to FD failed!" <<std::endl;
		return false;
	}

	if (backlog > 0){
		if (listen(listenerFd, backlog) == -1){
			std::cerr << "listen function call failed!" << std::endl;
			return false;
		}
	}
	else{
		std::cerr << "backlog invalid!" << std::endl;
		return false;
	}
	return true;
}

std::vector<int> setupListenerSockets(std::vector<Server> serverList){
	std::vector<int> listenerFdList;
	for (size_t i = 0; i < serverList.size(); i++){
		sockaddr_in socketAddr;
		std::cout << serverList[i].host << std::endl;
		if (toSockAddr(serverList[i].host, serverList[i].port, socketAddr) == false){
			throw (std::runtime_error("HOST/PORT invalid error!"));
		}
		int listenerFd = -1;
		if (openListeners(socketAddr, serverList[i].backlog, listenerFd) == false){
			throw (std::runtime_error("Listeners invalid error!"));
		}
		std::cout << "Listener Port [" << i + 1 << "] opened." << std::endl;
		listenerFdList.push_back(listenerFd);
	}
	if (listenerFdList.empty())
		throw (std::runtime_error("No Listeners Opened!"));
	std::cout << "Listeners are open... Waiting for response..." << std::endl;
	return (listenerFdList);
}
