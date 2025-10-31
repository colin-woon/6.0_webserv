#include "ServerLoop.hpp"
#include "PollHelpers.hpp"
#include "ServerHelpers.hpp"
#include "Timeout.hpp"
#include "../../http/HttpHandler.hpp"
#include "../../cgi/CGI.hpp"

Client::Client() : fd(-1), closeFlag(false), serverConfig(NULL), timeoutMs(0), expiresAtMs(0), responseQueued(false), headersDone(false), headerEndPos(0),
				isChunked(false), contentLength(0), parsePos(0), chunkRemain(0),chunkStage(0), consumedBytes(0) {}

Client::~Client(){}

ServerLoop::ServerLoop(const std::vector<int>& listenerFdList, const std::vector<Server>& serverList):listenerFdList_(listenerFdList), servers_(serverList)
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
	std::vector<CGIcontext>	cgiToClose;
	for (std::map<int, Client>::const_iterator it = clientList_.begin();
			it != clientList_.end(); ++it) {
		const Client& c = it->second;
		if (c.timeoutMs > 0 && c.expiresAtMs > 0 && now >= c.expiresAtMs)
			toClose.push_back(it->first);
	}
	for (std::map<int, CGIcontext>::iterator it = CGIMap_.begin();
			it != CGIMap_.end(); it++)
	{
		if (now >= it->second.expiresAtMs)
			cgiToClose.push_back(it->second);
	}
	for (size_t i = 0; i < toClose.size(); ++i)
		closeClient_(toClose[i]);
	for (size_t i = 0; i < cgiToClose.size(); ++i)
	{
		std::cout << "closed(CGI) " << cgiToClose[i].fd << std::endl;
		kill(cgiToClose[i].pid, SIGKILL);
		waitpid(cgiToClose[i].pid, NULL, 0);
		delPollFd(this->pollFdList_, this->fdIndex_, cgiToClose[i].fd);
		close(cgiToClose[i].fd);
		CGIMap_.erase(cgiToClose[i].fd);
	}
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
				if (CGIMap_.find(p.fd) != CGIMap_.end())
				{
					if ((p.revents & POLLHUP) && (p.revents & POLLIN))
						readOnceCGI_(CGIMap_[p.fd]);
					continue ;
				}
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
			if (CGIMap_.find(p.fd) != CGIMap_.end()) {
				if (p.revents & POLLOUT) readyWriters.push_back(p.fd);
				if (p.revents & POLLIN) readyReaders.push_back(p.fd);
			}
		}

		for (size_t i = 0; i < errFds.size(); ++i)
			closeClient_(errFds[i]);
		for (size_t i = 0; i < readyListeners.size(); ++i)
			acceptClients_(readyListeners[i]);
		for (size_t i = 0; i < readyWriters.size(); ++i)
		{
			if (clientList_.find(readyWriters[i]) != clientList_.end())
				writeOnce_(readyWriters[i]);
			else if (CGIMap_.find(readyWriters[i]) != CGIMap_.end())
				writeOnceCGI_(CGIMap_[readyWriters[i]]);
		}
		for (size_t i = 0; i < readyReaders.size(); ++i) {
			const int fd = readyReaders[i];
			if (clientList_.find(fd) != clientList_.end())
				readOnce_(fd);
			else if (CGIMap_.find(fd) != CGIMap_.end())
				readOnceCGI_(CGIMap_[fd]);
		}

		sweepExpiredClients();
	}
}

void	ServerLoop::addSocket(struct pollfd &pfd, pid_t pid, Client& client, int timeout)
{
	addPollFd(this->pollFdList_, this->fdIndex_, pfd.fd, POLLIN | POLLOUT);
	CGIcontext ctx;
	ctx.pid = pid;
	ctx.fd = pfd.fd;
	ctx.clientFd = client.fd;
	ctx.sendPos = 0;
	ctx.buffer.clear();
	ctx.expiresAtMs = nowMs() + timeout * 1000;
	ctx.response = client.response;
	this->CGIMap_[pfd.fd] = ctx;
	std::cout << "accepted(CGI) " << pfd.fd << std::endl;
}

void	ServerLoop::readOnceCGI_(CGIcontext& ctx)
{
	char buff[4096];

	ssize_t n = recv(ctx.fd, buff, sizeof(buff), 0);
	if (n > 0)
	{
		ctx.buffer.append(buff, n);
		return ;
	}
	else if (n == 0)
	{
		size_t separatorPos = ctx.buffer.find("\r\n\r\n");
		std::string cgiBody;
		if (separatorPos != std::string::npos)
		{
			std::string cgiHeadersStr = ctx.buffer.substr(0, separatorPos);
			cgiBody = ctx.buffer.substr(separatorPos + 4);
		
			std::stringstream ss(cgiHeadersStr);
			std::string headerLine;
			while (std::getline(ss, headerLine) && !headerLine.empty() && headerLine != "\r")
			{
				size_t colonPos = headerLine.find(":");
				if (colonPos != std::string::npos)
				{
					std::string key = headerLine.substr(0, colonPos);
					std::string value = headerLine.substr(colonPos + 2);
					if (!value.empty() && value[value.size() - 1] == '\r')
						value.erase(value.size() - 1);
					ctx.response.addHeader(key, value);
				}
			}
		}
		else
			cgiBody = ctx.buffer;
		ctx.response.setStatusCode(HttpException::statusCodeToString(HTTP_200_OK));
		ctx.response.setBody(cgiBody);
		if (ctx.response.getHeaders().find("Content-Length") == ctx.response.getHeaders().end())
		{
			std::stringstream contentLength;
			contentLength << cgiBody.length();
			ctx.response.addHeader("Content-Length", contentLength.str());
		}

		std::map<int, Client>::iterator it = clientList_.find(ctx.clientFd);
		if (it != clientList_.end())
		{
			Client& c = it->second;
			c.outBuff = ctx.response.toString();
			c.responseQueued = true;
			modifyEvent(pollFdList_, fdIndex_, ctx.clientFd, (short) (POLLOUT | POLLIN));
			c.response.isCGI = false;
		}

		std::cout << "close(CGI) " << ctx.fd << std::endl;
		delPollFd(this->pollFdList_, this->fdIndex_, ctx.fd);
		waitpid(ctx.pid, NULL, 0);
		close(ctx.fd);
		CGIMap_.erase(ctx.fd);
		return ;
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		std::cerr << "CGI Read Error on fd" << ctx.fd << std::endl;
		std::cout << "close(CGI) " << ctx.fd << std::endl;
		delPollFd(this->pollFdList_, this->fdIndex_, ctx.fd);
		kill(ctx.pid, SIGKILL);
		waitpid(ctx.pid, NULL, 0);
		close(ctx.fd);
		CGIMap_.erase(ctx.fd);
		return ;
	}
}

void	ServerLoop::writeOnceCGI_(CGIcontext& ctx)
{
	std::map<int, Client>::iterator it = clientList_.find(ctx.clientFd);
	std::string	data = it->second.request.getBody();
	if (ctx.sendPos >= data.size())
	{
		modifyEvent(this->pollFdList_, this->fdIndex_, ctx.fd, POLLIN);
		return ;
	}

	ssize_t n = send(ctx.fd, data.data() + ctx.sendPos, data.size() - ctx.sendPos, 0);
	if (n > 0)
	{
		ctx.sendPos += n;
		return ;
	}
	else if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		std::cerr << "CGI write error fd " << ctx.fd << " errno=" << errno << std::endl;
		std::cout << "close(CGI) " << ctx.fd << std::endl;
		delPollFd(this->pollFdList_, this->fdIndex_, ctx.fd);
		kill(ctx.pid, SIGKILL);
		waitpid(ctx.pid, NULL, 0);
		close(ctx.fd);
		CGIMap_.erase(ctx.fd);
		return ;
	}
}
