#include "PollHelpers.hpp"

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

