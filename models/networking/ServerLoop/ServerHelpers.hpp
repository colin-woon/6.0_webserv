#ifndef SERVERHELPERS_HPP
#define SERVERHELPERS_HPP

#include "ServerLoop.hpp"
#include "PollHelpers.hpp"

void closeClient(int fd, std::vector<pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clientList);
bool headersComplete(const std::string& s);
void readOnce(int fd, std::vector<struct pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clients);
void writeOnce(int fd, std::vector<struct pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clients);
void acceptClients(int lfd, std::vector<struct pollfd>& pollFdList, std::map<int, size_t>& fdIndex, std::map<int, Client>& clientList, std::map<int, int> &listenerTimeoutMs);
bool findHeadersEndAndMark(Client& c);
std::string toLowerCopy(const std::string& s);
void detectBodyFraming(Client& c);


#endif
