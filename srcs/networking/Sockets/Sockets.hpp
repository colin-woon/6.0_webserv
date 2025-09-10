#ifndef SOCKETS_HPP
#define SOCKETS_HPP

#include "../../../includes/server.hpp"

std::vector<int> setupListenerSockets(std::vector<Server> serverList);
bool setNonBlocking(int listenerFd);

#endif
