#ifndef SERVERLOOP_HPP
#define SERVERLOOP_HPP

#include "../../parsing/Server.hpp"
#include "../Sockets/Sockets.hpp"
#include <ostream>
#include <stdexcept>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <climits>

#include "../../http/HttpRequest.hpp"
#include "../../http/HttpResponse.hpp"
#include "../../http/HttpExceptions.hpp"
#include "../../http/HttpUtils.hpp"
#include "../../http/HttpHandlerGET.hpp"
#include "../../http/HttpHandler.hpp"

class Client
{
public:
	Client();
	~Client();
	int fd;
	std::string inBuff;
	std::string outBuff;
	bool closeFlag;
	const Server *owner;

	int timeoutMs;
	long long expiresAtMs;

	HttpRequest request;
	HttpResponse response;

	bool responseQueued;
};

void mainServerLoop(std::vector<int> &listenerFdList, std::vector<Server> &serverList);

#endif
