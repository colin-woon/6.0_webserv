#ifndef SERVERLOOP_HPP
#define SERVERLOOP_HPP

#include "../../parsing/Server.hpp"
#include "../Sockets/Sockets.hpp"
#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <climits>
#include <stdint.h>

#include "../../http/HttpRequest.hpp"
#include "../../http/HttpResponse.hpp"
// #include "../../http/HttpExceptions.hpp"
// #include "../../http/HttpUtils.hpp"
// #include "../../http/HttpHandlerGET.hpp"

class Client
{
public:
	Client();
	~Client();
	int fd;
	std::string inBuff;
	std::string outBuff;
	bool closeFlag;
	const Server *serverConfig;

	int timeoutMs;
	uint64_t expiresAtMs;

	HttpRequest request;
	HttpResponse response;
	bool responseQueued;
	bool headersDone;
	size_t headerEndPos;

	bool isChunked;
	size_t contentLength;

	size_t parsePos;
	size_t chunkRemain;
	int chunkStage; // 0=SIZE, 1=DATA, 2=CRLF, 3=TRAILERS, 4=DONE

	std::string bodyBuf; // decoded body (or stream to CGI stdin instead)
	size_t consumedBytes; // set to end-of-request when DONE
};

void mainServerLoop(std::vector<int> &listenerFdList, std::vector<Server> &serverList);

#endif
