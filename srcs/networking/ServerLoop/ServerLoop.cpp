#include "ServerLoop.hpp"
#include "PollHelpers.hpp"
#include "ServerHelpers.hpp"

Client::Client():fd(-1), closeFlag(false){}

Client::~Client(){}

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
		// std::cout << "Waiting for connection..." << std::endl;
		if (pollReady == 0)
			continue;
        std::vector<int> errFds;
        std::vector<int> readyListeners;
        std::vector<int> readyReaders;
        std::vector<int> readyWriters;

        size_t count = pollFdList.size();
        for (size_t i = 0; i < count; ++i) {
			// std::cout << "i am looping here" << std::endl;
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
            if (clientList.find(p.fd) != clientList.end()){
                if ((p.revents & POLLOUT) != 0)
                    readyWriters.push_back(p.fd);
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
