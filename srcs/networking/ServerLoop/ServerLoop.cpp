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

static void readToBuffer(int fd, std::vector<struct pollfd>& pfds, std::map<int, size_t>& fdIndex, std::map<int, Client>& clientList){
    Client& c = clientList[fd];
	std::ifstream in("ServerLoop/test.txt");

	std::string buff;
	std::string final;
	while (std::getline(in, buff))
		final.append(buff).append("\n");

    while (1){
        char buf[4096];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
		if (n > 0){
            c.inBuff.append(buf, (size_t)n);
			std::cout << c.inBuff << std::endl;
			//below testing html page
			//write(fd, final.c_str(), 15000);
            // check for \r\n\r\n?
        }
		else if (n == 0){
            closeClient(fd, pfds, fdIndex, clientList);
            return;
        }
		else{
            if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
            std::cerr << "recv error on fd " << fd << std::endl;;
            closeClient(fd, pfds, fdIndex, clientList);
            return;
        }
    }
}

void acceptClients(int lfd, std::vector<struct pollfd>& pfds, std::map<int, size_t>& fdIndex, std::map<int, Client>& clientList){
    while (1){
        int clientFd = accept(lfd, 0, 0);
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
            std::cerr << "accept error\n";
			break;
        }
        if (setNonBlocking(clientFd) == false) {
            close(clientFd);
			continue;
        }
        addPollFd(pfds, fdIndex, clientFd, POLLIN);
        Client c;
        c.fd = clientFd;
        clientList[clientFd] = c;
        std::cout << "accepted " << clientFd << "\n";
    }
}

void mainServerLoop(std::vector<int>& listenerFdList, std::vector<Server>& serverList) {
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
		
		size_t count = pollFdList.size();
		for (size_t i = 0; i < count; i++){
			pollfd p = pollFdList[i];

			if (listenerOrNot(listenerFdList, p.fd)){
				if ((p.revents & POLLIN) != 0)
					acceptClients(p.fd, pollFdList,fdIndex, clientList);
				continue;
			}

			if (clientList.find(p.fd) != clientList.end()){
				if (((p.revents & POLLIN) != 0)){
					readToBuffer(p.fd, pollFdList, fdIndex, clientList);
				}
			}
		}

	}
}
