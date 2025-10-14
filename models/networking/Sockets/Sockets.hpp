#ifndef SOCKETS_HPP
#define SOCKETS_HPP

#include "../../parsing/Server.hpp"

std::vector<int> setupListenerSockets(const std::vector<Server>& serverList, std::map<int, std::vector<const Server*> >& outListenerOwners);
bool setNonBlocking(int listenerFd);

#endif
