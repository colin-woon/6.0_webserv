#include "Sockets.hpp"
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>

static std::string makeBindKey(uint32_t ipv4Net, int port) {
	char ipText[INET_ADDRSTRLEN];
	std::memset(ipText, 0, sizeof(ipText));

	in_addr ipAddrStruct;
	ipAddrStruct.s_addr = ipv4Net;

	if (inet_ntop(AF_INET, &ipAddrStruct, ipText, sizeof(ipText)) == NULL) {
		std::ostringstream keyBuilder;
		keyBuilder << "0x"
					<< std::hex
					<< static_cast<unsigned long>(ntohl(ipv4Net))
					<< ":"
					<< std::dec
					<< port;
		return keyBuilder.str();
	}

	std::ostringstream keyBuilder;
	keyBuilder << ipText << ":" << port;
	return keyBuilder.str();
}

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

std::vector<int> setupListenerSockets(const std::vector<Server>& serverList, std::map<int, std::vector<const Server*> >& outListenerOwners)
{
	std::map<std::string, std::vector<const Server*> > bindGroups;
	for (size_t serverIdx = 0; serverIdx < serverList.size(); ++serverIdx) {
		const Server& serverRef = serverList[serverIdx];

		sockaddr_in bindSockAddr;
		if (toSockAddr(serverRef.host, serverRef.port, bindSockAddr) == false) {
			throw std::runtime_error("HOST/PORT invalid error!");
		}
		std::string bindKey = makeBindKey(bindSockAddr.sin_addr.s_addr, serverRef.port);
		bindGroups[bindKey].push_back(&serverRef);
	}

	std::vector<int> listenerFdList;

	std::map<std::string, std::vector<const Server*> >::const_iterator bindIt = bindGroups.begin();
	for (; bindIt != bindGroups.end(); ++bindIt) {
		const std::string& bindKey = bindIt->first;
		const std::vector<const Server*>& groupCandidates = bindIt->second;
		const Server* primaryServer = groupCandidates[0];

		sockaddr_in listenerSockAddr;
		if (toSockAddr(primaryServer->host, primaryServer->port, listenerSockAddr) == false) {
			throw std::runtime_error("Internal sockaddr rebuild failed");
		}

		int listenerFd = -1;
		if (openListeners(listenerSockAddr, primaryServer->backlog, listenerFd) == false)
			throw std::runtime_error("Failed to open listener");
		if (setNonBlocking(listenerFd) == false) {
			close(listenerFd);
			throw std::runtime_error("Failed to set non-blocking");
		}
		listenerFdList.push_back(listenerFd);
		outListenerOwners[listenerFd] = groupCandidates;
		std::cout << "Listener opened on " << bindKey << " for " << groupCandidates.size() << " server block(s)\n";
	}
	if (listenerFdList.empty()) {
		throw std::runtime_error("No listeners opened");
	}
	std::cout << "Listeners are open... Waiting for response...\n";
	return listenerFdList;
}