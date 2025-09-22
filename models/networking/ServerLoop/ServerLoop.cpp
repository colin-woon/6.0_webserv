#include "ServerLoop.hpp"
#include "PollHelpers.hpp"
#include "ServerHelpers.hpp"
#include "Timeout.hpp"
#include "../../http/HttpHandler.hpp"

Client::Client():fd(-1), closeFlag(false), serverConfig(NULL), timeoutMs(0), expiresAtMs(0), responseQueued(false), headersDone(false), headerEndPos(0),
				isChunked(false), contentLength(0), parsePos(0), chunkRemain(0),chunkStage(0), consumedBytes(0) {}

Client::~Client(){}

ServerLoop::ServerLoop(const std::vector<int>& listenerFdList, const std::vector<Server>& serverList):listenerFdList_(listenerFdList)
{
	buildTimeoutList(listenerFdList_, const_cast<std::vector<Server>&>(serverList), listenerTimeoutMs_);
	for (size_t i = 0; i < listenerFdList_.size(); ++i)
		addPollFd(pollFdList_, fdIndex_, listenerFdList_[i], POLLIN);
}

void ServerLoop::sweepExpiredClients(){
	if (clientList_.empty())
		return;
	const uint64_t now = nowMs();
	std::vector<int> toClose;
	for (std::map<int, Client>::const_iterator it = clientList_.begin();
			it != clientList_.end(); ++it) {
		const Client& c = it->second;
		if (c.timeoutMs > 0 && c.expiresAtMs > 0 && now >= c.expiresAtMs)
			toClose.push_back(it->first);
	}
	for (size_t i = 0; i < toClose.size(); ++i)
		closeClient_(toClose[i]);
}

void ServerLoop::run(){
	std::vector<int> errFds;
	std::vector<int> readyListeners;
	std::vector<int> readyReaders;
	std::vector<int> readyWriters;

	while (1){
		int timeoutMs = calcNextTimeout(clientList_, 1000);
		int pollReady = poll(&pollFdList_[0], pollFdList_.size(), timeoutMs);
		if (pollReady == -1) {
			if (errno == EINTR) continue;
			throw std::runtime_error("poll error.");
		}

		if (pollReady == 0) {
			sweepExpiredClients();
			continue;
		}

		errFds.clear();
		readyListeners.clear();
		readyReaders.clear();
		readyWriters.clear();
		const size_t count = pollFdList_.size();
		for (size_t i = 0; i < count; ++i) {
			struct pollfd p = pollFdList_[i];
			if (p.revents == 0) continue;

			if ((p.revents & POLLERR) || (p.revents & POLLHUP) || (p.revents & POLLNVAL)) {
				if (!listenerOrNot(listenerFdList_, p.fd))
					errFds.push_back(p.fd);
				else
					std::cerr << "listener error-ish revents on fd " << p.fd << "\n";
				continue;
			}
			if (listenerOrNot(listenerFdList_, p.fd)) {
				if (p.revents & POLLIN) readyListeners.push_back(p.fd);
				continue;
			}
			if (clientList_.find(p.fd) != clientList_.end()) {
				if (p.revents & POLLOUT) readyWriters.push_back(p.fd);
				if (p.revents & POLLIN)  readyReaders.push_back(p.fd);
			}
		}

		for (size_t i = 0; i < errFds.size(); ++i)
			closeClient_(errFds[i]);
		for (size_t i = 0; i < readyListeners.size(); ++i)
			acceptClients_(readyListeners[i]);
		for (size_t i = 0; i < readyWriters.size(); ++i)
			if (clientList_.find(readyWriters[i]) != clientList_.end())
				writeOnce_(readyWriters[i]);
		for (size_t i = 0; i < readyReaders.size(); ++i) {
			const int fd = readyReaders[i];
			if (clientList_.find(fd) != clientList_.end())
				readOnce_(fd);
		}

		sweepExpiredClients();
	}
}

