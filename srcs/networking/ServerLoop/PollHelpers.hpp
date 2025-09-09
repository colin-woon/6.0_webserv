#ifndef POLLHELPERS_HPP
#define POLLHELPERS_HPP

#include "ServerLoop.hpp"


size_t indexOf(std::map<int, size_t>& fdIndex, int fd);
void addPollFd(std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, int fd, short events);
void modifyEvent(std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, int fd, short events);
void delPollFd(std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, int fd);
bool listenerOrNot(std::vector<int>& listenerFdList, int fd);


#endif
