#include "ServerLoop.hpp"
#include <ostream>
#include <stdexcept>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <climits> 

Client::Client():fd(-1), closeFlag(false){}

Client::~Client(){}

size_t indexOf(std::map<int, size_t>& fdIndex, int fd){
	std::map<int, size_t>::iterator itr = fdIndex.find(fd);
	if (itr == fdIndex.end())
		return (SIZE_MAX);
	return (itr->second);
}

void addPollFd(std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, int fd, short events){
	struct pollfd pollFd;

	pollFd.fd = fd;
	pollFd.events = events;
	pollFd.revents = 0;
	pollFdList.push_back(pollFd);
	fdIndex[fd] = pollFdList.size() - 1;
}

void modifyEvent(std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, int fd, short events){
	size_t i = indexOf(fdIndex, fd);
	if (i != SIZE_MAX)
		pollFdList[i].events = events;
}

void delPollFd(std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, int fd){
	std::map<int, size_t>::iterator itr = fdIndex.find(fd);
	if (itr == fdIndex.end())
		return;

    size_t i = itr->second;
    size_t last = pollFdList.size() - 1;

    if (i != last) {
        pollFdList[i] = pollFdList[last];
        fdIndex[pollFdList[i].fd] = i;
    }
    pollFdList.pop_back();
    fdIndex.erase(itr);
}

bool listenerOrNot(std::vector<int>& listenerFdList, int fd){
	for (size_t i = 0; i < listenerFdList.size(); i++){
		if (fd == listenerFdList[i])
			return true;
	}
	return false;
}

static void closeClient(int fd, std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clientList){
    delPollFd(pollFdList, fdIndex, fd);
    std::map<int, Client>::iterator itr = clientList.find(fd);
    if (itr != clientList.end())
		clientList.erase(itr);
    close(fd);
    std::cout << "closed " << fd << "\n";
}

bool headersComplete(const std::string& s){
    return (s.find("\r\n\r\n") != std::string::npos);
}

void buildSimpleResponse(Client& c){
    const char* body = "Hello from webserv (poll)\n";
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: text/plain\r\n"
        << "Content-Length: " << std::strlen(body) << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    c.outBuff = oss.str();
    c.closeFlag = true;
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
        if (c.outBuff.empty() && headersComplete(c.inBuff)) {
            buildSimpleResponse(c);
            modifyEvent(pollFdList, fdIndex, fd, (short)(POLLIN | POLLOUT));
        }
        return;
    }
    closeClient(fd, pollFdList, fdIndex, clients);
}

void writeOnce(int fd, std::vector<struct pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clients){
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end())
        return;
    Client& c = it->second;
    if (c.outBuff.empty()) {
        modifyEvent(pollFdList, fdIndex, fd, POLLIN);
        return;
    }
	std::cout << "Http Response to be sent:\n" << c.outBuff << std::endl; 
    ssize_t n = send(fd, c.outBuff.data(), c.outBuff.size(), 0);
    if (n > 0) {
        c.outBuff.erase(0, (size_t)n);
        if (c.outBuff.empty()) {
            modifyEvent(pollFdList, fdIndex, fd, POLLIN);
            if (c.closeFlag)
                closeClient(fd, pollFdList, fdIndex, clients);
        }
        return;
    }
    closeClient(fd, pollFdList, fdIndex, clients);
}

void acceptClients(int lfd, std::vector<struct pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clientList){
    while (1){
        int clientFd = accept(lfd, 0, 0);
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
            std::cerr << "accept error\n";
			break;
        }
        if (setNonBlocking(clientFd) == false){
            close(clientFd);
			continue;
        }
        addPollFd(pollFdList, fdIndex, clientFd, POLLIN);
        Client c;
        c.fd = clientFd;
        clientList[clientFd] = c;
        std::cout << "accepted " << clientFd << "\n";
    }
}

void mainServerLoop(std::vector<int>& listenerFdList, std::vector<Server>& serverList){
	std::vector<pollfd> pollFdList;
	std::map<int, size_t> fdIndex;
	std::map<int, Client> clientList;
	(void)serverList;

	for (size_t i = 0; i < listenerFdList.size(); i++)
		addPollFd(pollFdList, fdIndex, listenerFdList[i], POLLIN);
	while (1){
		//later need to change timeout according to server block
		int pollReady = poll(&pollFdList[0], pollFdList.size(), 1000);
		if (pollReady == -1){
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll error.");
		}
		//timeout place below later
		if (pollReady == 0)
			continue;

        std::vector<int> errFds;
        std::vector<int> readyListeners;
        std::vector<int> readyReaders;
        std::vector<int> readyWriters;

        size_t count = pollFdList.size();
        for (size_t i = 0; i < count; ++i) {
            struct pollfd p = pollFdList[i];
            if (p.revents == 0)
				continue;
            if ((p.revents & POLLERR) != 0 || (p.revents & POLLHUP) != 0 || (p.revents & POLLNVAL) != 0){
				if (!listenerOrNot(listenerFdList, p.fd))
                    errFds.push_back(p.fd);
				else
                    std::cerr << "listener error-ish revents on fd " << p.fd << "\n";
                continue;
            }
            if (listenerOrNot(listenerFdList, p.fd)) {
                if ((p.revents & POLLIN) != 0)
                    readyListeners.push_back(p.fd);
                continue;
            }
            if (clientList.find(p.fd) != clientList.end()) {
                if ((p.revents & POLLOUT) != 0)
                    readyWriters.push_back(p.fd); // prioritize write
                if ((p.revents & POLLIN) != 0)
                    readyReaders.push_back(p.fd);
            }
        }
        for (size_t i = 0; i < errFds.size(); ++i)
            closeClient(errFds[i], pollFdList, fdIndex, clientList);
        for (size_t i = 0; i < readyListeners.size(); ++i)
            acceptClients(readyListeners[i], pollFdList, fdIndex, clientList);
        for (size_t i = 0; i < readyWriters.size(); ++i)
            if (clientList.find(readyWriters[i]) != clientList.end())
                writeOnce(readyWriters[i], pollFdList, fdIndex, clientList);
        for (size_t i = 0; i < readyReaders.size(); ++i) {
            int fd = readyReaders[i];
            if (clientList.find(fd) != clientList.end())
                readOnce(fd, pollFdList, fdIndex, clientList);
		}
	}
}
