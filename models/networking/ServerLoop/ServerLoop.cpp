#include "ServerLoop.hpp"
#include "PollHelpers.hpp"
#include "ServerHelpers.hpp"
#include "Timeout.hpp"
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
		if (now >= it->second.expiresAtMs ||
			std::find(toClose.begin(), toClose.end(), it->second.clientFd) != toClose.end())
			cgiToClose.push_back(it->second);
	}
	for (size_t i = 0; i < cgiToClose.size(); ++i)
	{
		CGIcontext& ctx = cgiToClose[i];
		try
		{
			throw Http504GatewayTimeoutException();
		}
		catch(const HttpException& e)
		{
			if (clientList_.find(ctx.clientFd) != clientList_.end())
			{
				Client &c = clientList_[ctx.clientFd];
				handleHttpException(*c.serverConfig, &ctx.loc_, e, ctx.response_);
				ctx.response_.addHeader("Connection", "close");
				c.closeFlag = true;
				c.outBuff = ctx.response_.toString();
				c.responseQueued = true;
				modifyEvent(pollFdList_, fdIndex_, c.fd, POLLOUT);
			}
		}
		closeCGI(ctx);
	}
	for (size_t i = 0; i < toClose.size(); ++i)
		closeClient_(toClose[i]);
}

void ServerLoop::run(){
	std::vector<int> errFds;
	std::vector<int> readyListeners;
	std::vector<int> readyReaders;
	std::vector<int> readyWriters;
	std::vector<int> touched;

	while (1){
		int timeoutMs = calcNextTimeout(clientList_, CGIMap_, 1000);
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
		touched.clear();
		const size_t count = pollFdList_.size();
		for (size_t i = 0; i < count; ++i) {
			struct pollfd p = pollFdList_[i];
			if (p.revents == 0) continue;

			if ((p.revents & POLLERR) || (p.revents & POLLHUP) || (p.revents & POLLNVAL)) {
				if (CGIbyRead_.find(p.fd) != CGIbyRead_.end() ||
						CGIbyWrite_.find(p.fd) != CGIbyWrite_.end())
				{
					if (p.revents & (POLLHUP | POLLIN))
						readyReaders.push_back(p.fd);
				}
				else if (!listenerOrNot(listenerFdList_, p.fd))
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
			if (CGIbyRead_.find(p.fd) != CGIbyRead_.end())
				if (p.revents & POLLIN) readyReaders.push_back(p.fd);
			if (CGIbyWrite_.find(p.fd) != CGIbyWrite_.end())
				if (p.revents & POLLOUT) readyWriters.push_back(p.fd);
		}
		for (size_t i = 0; i < errFds.size(); ++i)
			closeClient_(errFds[i]);
		for (size_t i = 0; i < readyListeners.size(); ++i)
			acceptClients_(readyListeners[i]);
		for (size_t i = 0; i < readyReaders.size(); ++i) {
			const int fd = readyReaders[i];
			if (clientList_.find(fd) != clientList_.end())
				readOnce_(fd);
			else if (CGIbyRead_.find(fd) != CGIbyRead_.end())
				readOnceCGI_(CGIbyRead_[fd]->second);
			else
				continue ;
			touched.push_back(fd);
		}
		for (size_t i = 0; i < readyWriters.size(); ++i)
		{
			const int fd = readyWriters[i];
			if (std::find(touched.begin(), touched.end(), fd) != touched.end())
				continue ;
			if (clientList_.find(fd) != clientList_.end())
				writeOnce_(fd);
			else if (CGIbyWrite_.find(fd) != CGIbyWrite_.end())
				writeOnceCGI_(CGIbyWrite_[fd]->second);
		}
		sweepExpiredClients();
	}
}

void	ServerLoop::addPollCGI(int ReadFd, int Writefd, pid_t pid, Client& client, Router& router)
{
	addPollFd(this->pollFdList_, this->fdIndex_, ReadFd, POLLIN);
	addPollFd(this->pollFdList_, this->fdIndex_, Writefd, POLLOUT);
	CGIcontext ctx(client, *router.locationConfig);
	ctx.pid = pid;
	ctx.fdRead = ReadFd;
	ctx.fdWrite = Writefd;
	ctx.sendPos = 0;
	ctx.buffer.clear();
	ctx.timeoutMs = router.locationConfig->cgi_timeout_sec * 1000;
	ctx.expiresAtMs = nowMs() + uint64_t(ctx.timeoutMs);
	ctx.srvConfig = client.serverConfig;
	std::map<int, CGIcontext>::iterator it = CGIMap_.insert(std::make_pair(pid, ctx)).first;
	this->CGIbyRead_[ReadFd] = it;
	this->CGIbyWrite_[Writefd] = it;
	std::cout << "accepted(CGI) [" << ReadFd << ", " << Writefd << "] on Client " << client.fd << std::endl;
}

void	ServerLoop::readOnceCGI_(CGIcontext &ctx)
{
	char buff[4096];

	ssize_t n = read(ctx.fdRead, buff, sizeof(buff));
	if (n > 0)
	{
		ctx.buffer.append(buff, n);
		return ;
	}
	else if (n == 0)
	{
		std::map<int, Client>::iterator it = clientList_.find(ctx.clientFd);
		it->second.serverConfig;
		if (it != clientList_.end())
		{
			CGI::getHttpResponse(ctx);
			Client& c = it->second;
			c.outBuff = ctx.response_.toString();
			c.responseQueued = true;
			modifyEvent(pollFdList_, fdIndex_, ctx.clientFd, (short) (POLLOUT | POLLIN));
		}
		waitpid(ctx.pid, NULL, WNOHANG);
		closeCGIRead(ctx);
		return ;
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		std::cerr << "CGI Read Error on fd " << ctx.fdRead << std::endl;
		try
		{
			throw Http500InternalServerErrorException();
		}
		catch(const HttpException& e)
		{
			if (clientList_.find(ctx.clientFd) != clientList_.end())
			{
				Client &c = clientList_[ctx.clientFd];
				handleHttpException(*c.serverConfig, &ctx.loc_, e, ctx.response_);
				ctx.response_.addHeader("Connection", "close");
				c.closeFlag = true;
				c.outBuff = ctx.response_.toString();
				c.responseQueued = true;
				modifyEvent(pollFdList_, fdIndex_, c.fd, POLLOUT);
			}
		}
		closeCGI(ctx);
		return ;
	}
}

void	ServerLoop::writeOnceCGI_(CGIcontext &ctx)
{
	std::map<int, Client>::iterator it = clientList_.find(ctx.clientFd);
	std::string	data = it->second.request.getBody();
	if (ctx.sendPos >= data.size())
	{
		closeCGIWrite(ctx);
		return ;
	}

	ssize_t n = write(ctx.fdWrite, data.data() + ctx.sendPos, data.size() - ctx.sendPos);
	if (n > 0)
	{
		ctx.sendPos += n;
		return ;
	}
	else if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		std::cerr << "CGI write error fd " << ctx.fdWrite << " errno=" << errno << std::endl;
		try
		{
			throw Http500InternalServerErrorException();
		}
		catch(const HttpException& e)
		{
			if (clientList_.find(ctx.clientFd) != clientList_.end())
			{
				Client &c = clientList_[ctx.clientFd];
				handleHttpException(*c.serverConfig, &ctx.loc_, e, ctx.response_);
				ctx.response_.addHeader("Connection", "close");
				c.closeFlag = true;
				c.outBuff = ctx.response_.toString();
				c.responseQueued = true;
				modifyEvent(pollFdList_, fdIndex_, c.fd, POLLOUT);
			}
		}
		closeCGI(ctx);
		return ;
	}
}

void	ServerLoop::closeCGIRead(CGIcontext &ctx)
{
	std::cout << "close(CGI Read) " << ctx.fdRead << std::endl;
	delPollFd(this->pollFdList_, this->fdIndex_, ctx.fdRead);
	close(ctx.fdRead);
	CGIbyRead_.erase(ctx.fdRead);
	if (CGIbyWrite_.find(ctx.fdWrite) == CGIbyWrite_.end())
		CGIMap_.erase(ctx.pid);
	return ;
}

void	ServerLoop::closeCGIWrite(CGIcontext &ctx)
{
	std::cout << "close(CGI Write) " << ctx.fdWrite << std::endl;
	delPollFd(this->pollFdList_, this->fdIndex_, ctx.fdWrite);
	close(ctx.fdWrite);
	CGIbyWrite_.erase(ctx.fdWrite);
	if (CGIbyRead_.find(ctx.fdRead) == CGIbyRead_.end())
		CGIMap_.erase(ctx.pid);
	return ;
}

void	ServerLoop::closeCGI(CGIcontext& ctx)
{
	kill(ctx.pid, SIGKILL);
	waitpid(ctx.pid, NULL, WNOHANG);
	if (CGIbyRead_.find(ctx.fdRead) != CGIbyRead_.end())
		closeCGIRead(ctx);
	if (CGIbyWrite_.find(ctx.fdWrite) != CGIbyWrite_.end())
		closeCGIWrite(ctx);
	CGIMap_.erase(ctx.pid);
	return ;
}
