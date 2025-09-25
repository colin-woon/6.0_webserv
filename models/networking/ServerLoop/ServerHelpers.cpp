
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
    size_t takenBytes = s.size() - pos;
    if (takenBytes > n) takenBytes = n;
    return s.substr(pos, takenBytes);
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
			if (errno == EAGAIN || errno == EWOULDBLOCK) break;
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
		
		std::cout << c.serverConfig->location[1].path << std::endl;;

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
			// c.request._header.assign(c.inBuff, 0, c.headerEndPos + 4); //implement? or no?
			c.request.setBody(c.bodyBuf);
			std::cout << c.request.getBody() << std::endl; //simple print to test chunked intake
			HttpHandler::handleRequest(c);
			c.outBuff = c.response.toString();
			c.responseQueued = true;
			modifyEvent(pollFdList_, fdIndex_, fd, (short)(POLLIN | POLLOUT));
			return;
		}
		else if (r < 0) {
			HttpHandler::handleRequest(c);
			c.outBuff = c.response.toString();
			std::cout << c.outBuff << std::endl;
			c.responseQueued = true;
			c.closeFlag = true;
			modifyEvent(pollFdList_, fdIndex_, fd, (short)(POLLIN | POLLOUT));
		}
		return;
	}

	c.consumedBytes = c.inBuff.size();
	// std::cout<<c.inBuff<<std::endl;
	HttpHandler::handleRequest(c);
	c.outBuff = c.response.toString();
	c.responseQueued = true;
	modifyEvent(pollFdList_, fdIndex_, fd, (short)(POLLIN | POLLOUT));
}

void ServerLoop::writeOnce_(int fd) {
	std::map<int, Client>::iterator it = clientList_.find(fd);
	if (it == clientList_.end()) return;
	Client &c = it->second;

	if (c.outBuff.empty()) {
		modifyEvent(pollFdList_, fdIndex_, fd, POLLIN);
		if (c.closeFlag) {
			std::cout << "closing, 2nd loop with no content" << std::endl;
			closeClient_(fd);
		} else {
			resetKeepAlive_(c);
			std::cout << "[KEEP-ALIVE] reset: inBuff.size=" << c.inBuff.size() << " headersDone=" << c.headersDone << " isChunked=" << c.isChunked << "\n"; // test print
		}
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
}

int ServerLoop::parseChunkSizeHex_(const std::string& s, size_t start, size_t end, size_t& outSz) {
	size_t v = 0;
	size_t i = start;
	if (start >= end)
		return 0;
	while (i < end) {
		char c = s[i];
		unsigned int d;
		if (c >= '0' && c <= '9')
			d = (unsigned int)(c - '0');
		else if (c >= 'a' && c <= 'f')
			d = 10u + (unsigned int)(c - 'a');
		else if (c >= 'A' && c <= 'F')
			d = 10u + (unsigned int)(c - 'A');
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
	int endofLine = c.inBuff.find("\r\n", c.parsePos);
	if (endofLine == std::string::npos)
		return 0;
	int sizeEnd = endofLine;
	size_t sz = 0;
	if (!parseChunkSizeHex_(c.inBuff, c.parsePos, sizeEnd, sz))
		return -1;

	c.chunkRemain = sz;
	c.parsePos = endofLine + 2; // move behind \r\n
	std::cout << "[CHUNKED] size-line: size=" << c.chunkRemain << " parsePos=" << c.parsePos << " nextStage=" << (c.chunkRemain == 0 ? 3 : 1) << "\n"; //test print
	if (sz == 0)
		c.chunkStage = 3;
	else
		c.chunkStage = 1;
	return 1;
}

int ServerLoop::stepCopyChunkData_(Client& c, size_t maxBody) {
	if (c.parsePos >= c.inBuff.size())
		return 0;

	size_t availBytes = c.inBuff.size() - c.parsePos; //how many bytes available in the buffer
	size_t takenBytes = c.chunkRemain; //how many bytes in the chunk as stated from the hexadecimal value check
	if (takenBytes > availBytes) //if client sent incomplete content in the buffer, reduce takenbytes so i can poll again and get the remainder
		takenBytes = availBytes;

	if (takenBytes > 0) {
		c.bodyBuf.append(c.inBuff, c.parsePos, takenBytes);
		c.parsePos += takenBytes;
		c.chunkRemain -= takenBytes;
	if (maxBody > 0) {
		if (c.bodyBuf.size() > maxBody)
			return -1;
	}
	}
	if (c.chunkRemain > 0) // incomplete, wait for next poll loop to complete the chunk
		return 0;
	c.chunkStage = 2; // complete chunk, go to next stage
	return 1;
}

int ServerLoop::stepCheckLineCRLF_(Client& c) {
	if (c.parsePos + 2 > c.inBuff.size()) //check if parsepos + 2 (\r\n) stays within bound of actual buffer
		return 0;
	if (c.inBuff[c.parsePos] != '\r' || c.inBuff[c.parsePos + 1] != '\n') // check if \r\n is at the end of current line
		return -1;
	c.parsePos += 2;
	c.chunkStage = 0;
	return 1;
}

int ServerLoop::stepConsumeTrailers_(Client& c) {
	if (c.parsePos + 2 <= c.inBuff.size()) { // check if only 2 bytes at the end in the case where client do not send trailer
		if (c.inBuff[c.parsePos] == '\r' && c.inBuff[c.parsePos + 1] == '\n') {
			c.parsePos += 2;
			c.chunkStage = 4;// done
			return 1;
		}
	}
	std::string::size_type end = c.inBuff.find("\r\n\r\n", c.parsePos); //client sends trailer
	if (end == std::string::npos){
		return 0;
	}
	c.parsePos = end + 4;
	c.chunkStage = 4;
	return 1;
}

int ServerLoop::unchunkStep_(Client& c, size_t maxBody) {
	while (1) {
		if (c.chunkStage == 0) {
			int r = stepReadSizeLine_(c); //parses line size, moves pointer to eol (behind \r\n)
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
			return -1;
	}
}

