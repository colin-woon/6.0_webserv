#include "ServerHelpers.hpp"
#include "Timeout.hpp"
#include "../../http/HttpHandler.hpp"

void closeClient(int fd, std::vector<pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clientList)
{
	delPollFd(pollFdList, fdIndex, fd);
	std::map<int, Client>::iterator itr = clientList.find(fd);
	if (itr != clientList.end())
		clientList.erase(itr);
	close(fd);
	std::cout << "closed " << fd << "\n";
}

static void resetKeepAlive(Client& c) {
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


static int parseChunkSizeFrom(const std::string& s, size_t start, size_t end, size_t& outSz) {
	size_t v = 0;
	size_t i = start;
	if (start >= end) return 0;
	while (i < end) {
		char c = s[i];
		unsigned d;
		if (c >= '0' && c <= '9') d = (unsigned)(c - '0');
		else if (c >= 'a' && c <= 'f') d = 10u + (unsigned)(c - 'a');
		else if (c >= 'A' && c <= 'F') d = 10u + (unsigned)(c - 'A');
		else break;
		if (v > (SIZE_MAX - d) / 16u) return 0; // overflow guard
		v = v * 16u + d;
		++i;
	}
	if (i == start) return 0; // no hex digits
	outSz = v;
	return 1;
}

static int stepReadSizeLine(Client& c) {
	std::string::size_type eol = c.inBuff.find("\r\n", c.parsePos);
	if (eol == std::string::npos) return 0; // need more data

	// allow chunk extensions: "HEX;ext=val"
	std::string::size_type semi = c.inBuff.find(';', c.parsePos);
	std::string::size_type sizeEnd = eol;
	if (semi != std::string::npos && semi < eol)
		sizeEnd = semi;

	size_t sz = 0;
	if (!parseChunkSizeFrom(c.inBuff, c.parsePos, sizeEnd, sz))
		return -1; // bad hex

	c.chunkRemain = sz;
	c.parsePos = eol + 2;

	if (sz == 0)
		c.chunkStage = 3;  // trailers
	else
		c.chunkStage = 1;  // data
	return 1;
}

static int stepCopyChunkData(Client& c, size_t maxBody) {
	if (c.parsePos >= c.inBuff.size()) return 0;

	size_t avail = c.inBuff.size() - c.parsePos;
	size_t take = c.chunkRemain;
	if (take > avail) take = avail;

	if (take > 0) {
		// Option A (buffer): append to bodyBuf
		c.bodyBuf.append(c.inBuff, c.parsePos, take);

		// Option B (CGI): write to CGI stdin here instead of buffering.

		c.parsePos     += take;
		c.chunkRemain  -= take;

		if (maxBody > 0) {
			if (c.bodyBuf.size() > maxBody) return -1;
		}
	}

	if (c.chunkRemain > 0) return 0; // still need more bytes for this chunk
	c.chunkStage = 2;                 // expect CRLF after data
	return 1;
}

static int stepExpectDataCRLF(Client& c) {
	if (c.parsePos + 2 > c.inBuff.size()) return 0;
	if (c.inBuff[c.parsePos] != '\r' || c.inBuff[c.parsePos + 1] != '\n') return -1;
	c.parsePos += 2;
	c.chunkStage = 0; // back to read next size
	return 1;
}

static int stepConsumeTrailers(Client& c) {
	// Accept just a blank line (\r\n)
	if (c.parsePos + 2 <= c.inBuff.size()) {
		if (c.inBuff[c.parsePos] == '\r' && c.inBuff[c.parsePos + 1] == '\n') {
			c.parsePos += 2;
			c.chunkStage = 4;  // done
			return 1;
		}
	}
	// Or accept actual trailers ending with \r\n\r\n
	std::string::size_type end = c.inBuff.find("\r\n\r\n", c.parsePos);
	if (end == std::string::npos) return 0; // need more
	c.parsePos = end + 4;
	c.chunkStage = 4;
	return 1;
}

static int unchunkStep(Client& c, size_t maxBody) {
	while (1) {
		if (c.chunkStage == 0) {
			int r = stepReadSizeLine(c);
			if (r <= 0) return r;
		}
		else if (c.chunkStage == 1) {
			int r = stepCopyChunkData(c, maxBody);
			if (r <= 0) return r;
		}
		else if (c.chunkStage == 2) {
			int r = stepExpectDataCRLF(c);
			if (r <= 0) return r;
		}
		else if (c.chunkStage == 3) {
			int r = stepConsumeTrailers(c);
			if (r <= 0) return r;
		}
		else if (c.chunkStage == 4) {
			c.consumedBytes = c.parsePos; // end of this request in inBuff
			return 1;
		}
		else {
			return -1; // unknown stage
		}
	}
}

void readOnce(int fd, std::vector<struct pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clients){
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end())
		return;

	Client& c = it->second;

	char buf[4096];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);

	if (n > 0) {
		c.inBuff.append(buf, (size_t)n);
		resetClientTimeout(c);
		if (c.responseQueued)
			return;
		if (!c.headersDone) {
			if (!findHeadersEndAndMark(c))
				return;
			detectBodyFraming(c);
		}

		bool ready = false;

		if (c.isChunked) {
			int r = unchunkStep(c, 0);
			if (r > 0) {
				// Request complete: body is in c.bodyBuf (or was streamed to CGI).
				// If using CGI, close CGI stdin here to signal EOF.
				// std::cout << c.bodyBuf << std::endl;
				HttpHandler::handleRequest(c);
				c.outBuff = c.response.toString();
				c.responseQueued = true;
				modifyEvent(pollFdList, fdIndex, fd, (short)(POLLIN | POLLOUT));
				return;
			}
			if (r < 0) {
				HttpHandler::handleRequest(c);
				c.outBuff = c.response.toString();
				c.responseQueued = true;
				c.closeFlag = true;
				modifyEvent(pollFdList, fdIndex, fd, (short)(POLLIN | POLLOUT));
				return;
			}
			return;
		}
		else {
			if (c.contentLength == 0) {
				c.consumedBytes = c.headerEndPos + 4;
				ready = true;
			} else {
				size_t have = 0;
				if (c.inBuff.size() > c.headerEndPos + 4) {
					have = c.inBuff.size() - (c.headerEndPos + 4);
				}
				if (have >= c.contentLength) {
					c.consumedBytes = c.headerEndPos + 4 + c.contentLength;
					ready = true;
				}
			}
		}
		if (ready) {
			// std::cout << c.inBuff << std::endl;
			HttpHandler::handleRequest(c); // handler reads from c.inBuff as usual
			c.outBuff = c.response.toString();
			c.responseQueued = true;
			modifyEvent(pollFdList, fdIndex, fd, (short)(POLLIN | POLLOUT));
		}
		return;
	}
	closeClient(fd, pollFdList, fdIndex, clients);
}

void writeOnce(int fd, std::vector<struct pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clients)
{
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end())
		return;
	Client &c = it->second;
	if (c.outBuff.empty()) {
		modifyEvent(pollFdList, fdIndex, fd, POLLIN);
		if (c.closeFlag) {
			std::cout << "closing, 2nd loop with no content" << std::endl;
			closeClient(fd, pollFdList, fdIndex, clients);
		}
		else
			resetKeepAlive(c);
		return;
	}
	// std::cout << "Http Response to be sent by client: " << c.fd << "\n" << c.outBuff << std::endl;
	ssize_t n = send(fd, c.outBuff.data(), c.outBuff.size(), 0);
	if (n > 0)
	{
		c.outBuff.erase(0, (size_t)n);
		resetClientTimeout(c);
		if (c.outBuff.empty())
		{
			modifyEvent(pollFdList, fdIndex, fd, POLLIN);
			if (c.closeFlag)
			{
				std::cout << "closing, 2nd loop with no content" << std::endl;
				closeClient(fd, pollFdList, fdIndex, clients);
			}
			else
				resetKeepAlive(c);
		}
		return;
	}
	closeClient(fd, pollFdList, fdIndex, clients);
}

void acceptClients(int lfd, std::vector<struct pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clientList, std::map<int, int> &listenerTimeoutMs)
{
	while (1)
	{
		int clientFd = accept(lfd, 0, 0);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			std::cerr << "accept error\n";
			break;
		}
		if (setNonBlocking(clientFd) == false)
		{
			close(clientFd);
			continue;
		}
		addPollFd(pollFdList, fdIndex, clientFd, POLLIN);
		Client c;
		c.fd = clientFd;
		setClientTimeout(lfd, c, listenerTimeoutMs);
		clientList[clientFd] = c;
		std::cout << "accepted " << clientFd << "\n";
	}
}

bool findHeadersEndAndMark(Client& c){
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

std::string toLowerCopy(const std::string& s){
	std::string t = s;
	for (size_t i = 0; i < t.size(); ++i)
		t[i] = (char)std::tolower(t[i]);
	return t;
}

void detectBodyFraming(Client& c){
	c.isChunked = false;
	c.contentLength = 0;

	const std::string raw = c.inBuff.substr(0, c.headerEndPos + 4);
	std::string lower = toLowerCopy(raw);

	const std::string kCL = "content-length:";
	std::string::size_type q = lower.find(kCL);
	if (q != std::string::npos) {
		q += kCL.size();
		while (q < lower.size() && (lower[q] == ' ' || lower[q] == '\t')) ++q;
		size_t start = q;
		while (q < lower.size() && std::isdigit((unsigned char)lower[q])) ++q;
		if (q > start) {
			std::istringstream iss(lower.substr(start, q - start));
			size_t v = 0;
			if (iss >> v) c.contentLength = v;
		}
	}

	const std::string kTE = "transfer-encoding:";
	std::string::size_type t = lower.find(kTE);
	if (t != std::string::npos) {
		t += kTE.size();
		std::string teval = lower.substr(t, lower.find("\r\n", t) - t);
		if (teval.find("chunked") != std::string::npos)
			c.isChunked = true;
	}
}
