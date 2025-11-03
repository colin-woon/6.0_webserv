#ifndef TIMEOUT_HPP
#define TIMEOUT_HPP

#include "ServerLoop.hpp"
#include "PollHelpers.hpp"
#include "ServerHelpers.hpp"
#include <sys/time.h>

uint64_t nowMs();
void buildTimeoutList(std::vector<int>& listenerFdList, std::vector<Server>& serverList, std::map<int,int>& listenerTimeoutMs);
int calcNextTimeout(std::map<int,Client>& clients, std::map<int, CGIcontext>& CGImap, int fallbackMs);
void setClientTimeout(int listenerFd, Client& c, std::map<int,int>& listenerTimeoutMs);
void resetClientTimeout(Client& c);

#endif
