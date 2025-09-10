#ifndef SOCKETS_HPP
#define SOCKETS_HPP

#include "../../parsing/Server.hpp"

std::vector<int> setupListenerSockets(std::vector<Server> serverList);
bool setNonBlocking(int listenerFd);

#endif
