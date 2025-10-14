#ifndef SERVER_LOOP_HPP
#define SERVER_LOOP_HPP

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

class Client{
public:
	int				fd;
	bool			closeFlag;
	const Server*	serverConfig;
	int				timeoutMs;
	uint64_t		expiresAtMs;
	std::vector<const Server*> vhostCandidates;

	HttpRequest				request;
	HttpResponse			response;
	bool					responseQueued;
	bool					headersDone;
	std::string::size_type	headerEndPos;
	bool					isChunked;
	size_t					contentLength;
	std::string				inBuff;
	std::string				outBuff;
	std::string				bodyBuf;

	size_t	parsePos;
	size_t	chunkRemain;
	int		chunkStage;
	size_t	consumedBytes;

	Client();
	~Client();
};

class ServerLoop{
public:
	ServerLoop(const std::vector<int>& listenerFdList, const std::vector<Server>& serverList);
	std::vector<int> listenerFdList_;
	void run();
	void sweepExpiredClients();

	std::vector<struct pollfd>		pollFdList_;
	std::vector<Server>				servers_;
	std::map<int, size_t>			fdIndex_;
	std::map<int, Client>			clientList_;
	std::map<int, int>				listenerTimeoutMs_; // listener fd -> timeout ms
	std::map<int, std::vector<const Server*> > listenerOwner_;

	void closeClient_(int fd);
	void acceptClients_(int lfd);
	void readOnce_(int fd);
	void writeOnce_(int fd);

	static void resetKeepAlive_(Client& c);
	static bool findHeadersEndAndMark_(Client& c);
	static void detectBodyFraming_(Client& c);

	static int	parseChunkSizeHex_(const std::string& s, size_t start, size_t end, size_t& outSz);
	static int	parseContentLength_(const std::string& s, size_t start, size_t end, size_t& out);
	static int	stepReadSizeLine_(Client& c);
	static int	stepCopyChunkData_(Client& c, size_t maxBody);
	static int	stepCheckLineCRLF_(Client& c);
	static int	stepConsumeTrailers_(Client& c);
	static int	unchunkStep_(Client& c, size_t maxBody);
	const Server* vhostPicker_(const std::vector<const Server*>& candidates, const std::string& rawHeaders);

	static std::string toLowerCopy_(const std::string& s);
};

#endif
