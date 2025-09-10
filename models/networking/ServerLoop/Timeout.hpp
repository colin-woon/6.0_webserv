#ifndef TIMEOUT_HPP
#define TIMEOUT_HPP

#include "ServerLoop.hpp"
#include "PollHelpers.hpp"
#include "ServerHelpers.hpp"
#include <sys/time.h>


void buildTimeoutList(std::vector<int>& listenerFdList, std::vector<Server>& serverList, std::map<int,int>& listenerTimeoutMs);
int calcNextTimeout(std::map<int,Client>& clients, int fallbackMs);
void setClientTimeout(int listenerFd, Client& c, std::map<int,int>& listenerTimeoutMs);
void resetClientTimeout(Client& c);
void closeExpiredClients(std::vector<struct pollfd>& pollFdList, std::map<int,size_t>& fdIndex, std::map<int,Client>& clientList);

#endif
