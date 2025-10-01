#include "ServerLoop.hpp"
#include "PollHelpers.hpp"
#include "ServerHelpers.hpp"
#include "Timeout.hpp"
#include "../../http/HttpHandler.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

static std::string preview(const std::string& s, size_t pos, size_t n) {
	if (pos >= s.size()) return "";
	size_t take = s.size() - pos;
	if (take > n) take = n;
	return s.substr(pos, take);
}

void ServerLoop::closeClient_(int fd) {
	delPollFd(pollFdList_, fdIndex_, fd);
	std::map<int, Client>::iterator itr = clientList_.find(fd);
	if (itr != clientList_.end())
		clientList_.erase(itr);
	close(fd);
	std::cout << "closed " << fd << "\n";
}

void ServerLoop::acceptClients_(int lfd) {
	while (1){
		int clientFd = accept(lfd, 0, 0);
		if (clientFd == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			std::cerr << "accept error\n";
			break;
		}
		if (!setNonBlocking(clientFd)) {
			close(clientFd);
			continue;
		}
		addPollFd(pollFdList_, fdIndex_, clientFd, POLLIN);
		Client c;
		c.fd = clientFd;

		std::map<int,const Server*>::const_iterator it = listenerOwner_.find(lfd);
		if (it != listenerOwner_.end())
			c.serverConfig = it->second;
		else
			c.serverConfig = NULL;
		setClientTimeout(lfd, c, listenerTimeoutMs_);
		clientList_[clientFd] = c;
		std::cout << "accepted " << clientFd << "\n";
	}
}

void ServerLoop::readOnce_(int fd) {
	std::map<int, Client>::iterator it = clientList_.find(fd);
	if (it == clientList_.end())
		return;
	Client& c = it->second;

	char buf[4096];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);
	if (n < 0)
		return;

	c.inBuff.append(buf, (size_t)n);
	resetClientTimeout(c);
	if (c.responseQueued)
		return;

	if (!c.headersDone){
		if (!findHeadersEndAndMark_(c))
			return;
		detectBodyFraming_(c);
	}

	if (c.isChunked) {
		int r = unchunkStep_(c, 0);
		if (r > 0) {
			HttpHandler::handleRequest(c);
			c.outBuff = c.response.toString();
			c.responseQueued = true;
			modifyEvent(pollFdList_, fdIndex_, fd, (short)(POLLIN | POLLOUT));
			return;
		}
		else if (r < 0) {
			HttpHandler::handleRequest(c);
			c.outBuff = c.response.toString();
			c.responseQueued = true;
			c.closeFlag = true;
			modifyEvent(pollFdList_, fdIndex_, fd, (short)(POLLIN | POLLOUT));
		}
		return;
	}
	if (c.contentLength > 0){
		size_t bodyAvail;
		if (c.inBuff.size() >= c.parsePos)
			bodyAvail = c.inBuff.size() - c.parsePos;
		else
			bodyAvail = 0;
		if (bodyAvail < c.contentLength) //body not complete, go to next recv
			return;
		c.bodyBuf.assign(c.inBuff, c.parsePos, c.contentLength);
		c.consumedBytes = c.parsePos + c.contentLength;
	}
	else
		c.consumedBytes = c.parsePos;
	HttpHandler::handleRequest(c);
	c.outBuff = c.response.toString();
	c.responseQueued = true;
	modifyEvent(pollFdList_, fdIndex_, fd, (short)(POLLIN | POLLOUT));
}

void ServerLoop::writeOnce_(int fd) {
	std::map<int, Client>::iterator it = clientList_.find(fd);
	if (it == clientList_.end())
	return;
	Client &c = it->second;

	if (c.outBuff.empty()) {
		modifyEvent(pollFdList_, fdIndex_, fd, POLLIN);
		if (c.closeFlag) {
			std::cout << "closing, 2nd loop with no content" << std::endl;
			closeClient_(fd);
		} else
			resetKeepAlive_(c);
		return;
	}

	ssize_t n = send(fd, c.outBuff.data(), c.outBuff.size(), 0);
	if (n > 0) {
		c.outBuff.erase(0, (size_t)n);
		resetClientTimeout(c);
		if (c.outBuff.empty()) {
			modifyEvent(pollFdList_, fdIndex_, fd, POLLIN);
			if (c.closeFlag) {
				std::cout << "closing, 2nd loop with no content" << std::endl;
				closeClient_(fd);
			} else {
				resetKeepAlive_(c);
			}
		}
		return;
	}
	closeClient_(fd);
}

void ServerLoop::resetKeepAlive_(Client& c) {
	if (c.consumedBytes > 0 && c.consumedBytes <= c.inBuff.size())
		c.inBuff.erase(0, c.consumedBytes);
	c.headersDone = false;
	c.headerEndPos = 0;
	c.isChunked = false;
	c.contentLength = 0;
	c.consumedBytes = 0;
	c.parsePos = 0;
	c.chunkRemain = 0;
	c.chunkStage = 0;
	c.bodyBuf.clear();
	c.responseQueued = false;
	c.closeFlag = false;
}

bool ServerLoop::findHeadersEndAndMark_(Client& c) {
	if (c.headersDone)
		return true;
	std::string::size_type p = c.inBuff.find("\r\n\r\n");
	if (p == std::string::npos)
		return false;
	c.headersDone = true;
	c.headerEndPos = p;
	c.parsePos = p + 4;
	return true;
}

std::string ServerLoop::toLowerCopy_(const std::string& s) {
	std::string t = s;
	for (size_t i = 0; i < t.size(); ++i)
		t[i] = (char)std::tolower((unsigned char)t[i]);
	return t;
}

void ServerLoop::detectBodyFraming_(Client& c) {
	c.isChunked = false;
	c.contentLength = 0;

	const std::string raw = c.inBuff.substr(0, c.headerEndPos + 4);
	std::string lower = toLowerCopy_(raw);

	const std::string kTE = "transfer-encoding:";
	std::string::size_type t = lower.find(kTE);
	if (t != std::string::npos) {
		t += kTE.size();
		std::string teval = lower.substr(t, lower.find("\r\n", t) - t);
		if (teval.find("chunked") != std::string::npos)
			c.isChunked = true;
	}

	const std::string kCL = "content-length:";
	std::string::size_type p = lower.find(kCL);
	if (p != std::string::npos) {
		p += kCL.size();
		while (p < lower.size() && (lower[p] == ' ' || lower[p] == '\t'))
			++p;
		std::string::size_type eol = lower.find("\r\n", p);
		if (eol == std::string::npos)
			eol = lower.size();
		std::string::size_type end = eol;
		while (end > p && (lower[end - 1] == ' ' || lower[end - 1] == '\t'))
			--end;
		size_t cl = 0;
		if (parseContentLength_(lower, p, end, cl)) {
			c.contentLength = cl;
		}
		else
			// bad contentlength,treat as no body? maybe set 400 bad request?
			c.contentLength = 0;
	}
}

int ServerLoop::parseContentLength_(const std::string& s, size_t start, size_t end, size_t& out){
	if (start >= end)
		return 0;
	std::string tmp = s.substr(start, end - start);
	if (tmp.empty())
		return 0;

	for (size_t i = 0; i < tmp.size(); i++)
		if(tmp[i] < '0' || tmp[i] > '9')
			return 0;
	errno = 0;
	char *endptr = 0;
	unsigned long value = std::strtoul(tmp.c_str(), &endptr, 10);
	if (errno == ERANGE || endptr == tmp.c_str() || *endptr != '\0')
		return 0;
	out = static_cast<size_t>(value);
	return 1;
}

int ServerLoop::parseChunkSizeHex_(const std::string& s, size_t start, size_t end, size_t& outSz) {
	size_t v = 0;
	size_t i = start;
	if (start >= end)
		return 0;
	while (i < end) {
		char c = s[i];
		unsigned d;
		if (c >= '0' && c <= '9')
			d = (unsigned)(c - '0');
		else if (c >= 'a' && c <= 'f')
			d = 10u + (unsigned)(c - 'a');
		else if (c >= 'A' && c <= 'F')
			d = 10u + (unsigned)(c - 'A');
		else
			break;
		if (v > (SIZE_MAX - d) / 16u)
			return 0;
		v = v * 16u + d;
		++i;
	}
	if (i == start)
		return 0;
	outSz = v;
	return 1;
}

int ServerLoop::stepReadSizeLine_(Client& c) {
	std::string::size_type eol = c.inBuff.find("\r\n", c.parsePos);
	if (eol == std::string::npos)
		return 0;
	std::string::size_type semi = c.inBuff.find(';', c.parsePos);
	std::string::size_type sizeEnd = eol;
	if (semi != std::string::npos && semi < eol)
		sizeEnd = semi;
	size_t sz = 0;
	if (!parseChunkSizeHex_(c.inBuff, c.parsePos, sizeEnd, sz))
		return -1;

	c.chunkRemain = sz;
	c.parsePos = eol + 2;
	if (sz == 0)
		c.chunkStage = 3;
	else
		c.chunkStage = 1;
	return 1;
}

int ServerLoop::stepCopyChunkData_(Client& c, size_t maxBody) {
	if (c.parsePos >= c.inBuff.size()) return 0;

	size_t avail = c.inBuff.size() - c.parsePos;
	size_t take = c.chunkRemain;
	if (take > avail) take = avail;

	if (take > 0) {
		c.bodyBuf.append(c.inBuff, c.parsePos, take);
		c.parsePos += take;
		c.chunkRemain -= take;
		if (maxBody > 0) {
			if (c.bodyBuf.size() > maxBody){
				return -1;
			}
		}
	}

	if (c.chunkRemain > 0)
		return 0;
	c.chunkStage = 2;
	return 1;
}

int ServerLoop::stepCheckLineCRLF_(Client& c){
	if (c.parsePos + 2 > c.inBuff.size())
		return 0;
	if (c.inBuff[c.parsePos] != '\r' || c.inBuff[c.parsePos + 1] != '\n')
		return -1;
	c.parsePos += 2;
	c.chunkStage = 0;
	return 1;
}

int ServerLoop::stepConsumeTrailers_(Client& c) {
	if (c.parsePos + 2 <= c.inBuff.size()) {
		if (c.inBuff[c.parsePos] == '\r' && c.inBuff[c.parsePos + 1] == '\n') {
			c.parsePos += 2;
			c.chunkStage = 4;  // done
			return 1;
		}
	}
	std::string::size_type end = c.inBuff.find("\r\n\r\n", c.parsePos);
	if (end == std::string::npos)
		return 0;
	c.chunkStage = 4;
	return 1;
}

int ServerLoop::unchunkStep_(Client& c, size_t maxBody) {
	while (1) {
		if (c.chunkStage == 0) {
			int r = stepReadSizeLine_(c);
			if (r <= 0)
				return r;
		} else if (c.chunkStage == 1) {
			int r = stepCopyChunkData_(c, maxBody);
			if (r <= 0)
				return r;
		} else if (c.chunkStage == 2) {
			int r = stepCheckLineCRLF_(c);
			if (r <= 0)
				return r;
		} else if (c.chunkStage == 3) {
			int r = stepConsumeTrailers_(c);
			if (r <= 0)
				return r;
		} else if (c.chunkStage == 4) {
			c.consumedBytes = c.parsePos; // end of request in inBuff
			return 1;
		} else
			return -1; // unknown stage
	}
}

