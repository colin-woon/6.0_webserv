#ifndef SERVERLOOP_HPP
#define SERVERLOOP_HPP

#include "../../../includes/Server.hpp"
#include "../Sockets/Sockets.hpp"
#include <ostream>
#include <stdexcept>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <climits> 


#include "../../http/models/HttpRequest.hpp"
#include "../../http/models/HttpResponse.hpp"
#include "../../http/models/HttpExceptions.hpp"
#include "../../http/models/HttpUtils.hpp"
#include "../../http/models/HttpHandlerGET.hpp"
#include "../../http/models/HttpHandler.hpp"

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

		HttpRequest request;
		HttpResponse response;

		bool responseQueued;
};

void mainServerLoop(std::vector<int>& listenerFdList, std::vector<Server>& serverList);

#endif

