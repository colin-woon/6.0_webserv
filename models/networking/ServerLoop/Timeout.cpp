#include "Timeout.hpp"

long long nowMs()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	long long ms = (long long)tv.tv_sec * 1000;
	ms += (long long)(tv.tv_usec / 1000);
	return (ms);
}

void buildTimeoutList(std::vector<int> &listenerFdList, std::vector<Server> &serverList, std::map<int, int> &listenerTimeoutMs)
{
	listenerTimeoutMs.clear();
	size_t n = listenerFdList.size();
	if (serverList.size() < n)
		n = serverList.size();
	for (size_t i = 0; i < n; ++i)
	{
		int ms = 0;
		if (serverList[i].client_timeout_sec > 0)
		{
			long long tmp = (long long)serverList[i].client_timeout_sec * 1000;
			if (tmp > INT_MAX)
				ms = INT_MAX;
			else
				ms = (int)tmp;
		}
		listenerTimeoutMs[listenerFdList[i]] = ms;
	}
}

void setClientTimeout(int listenerFd, Client &c, std::map<int, int> &listenerTimeoutMs)
{
	c.timeoutMs = 0;
	c.expiresAtMs = 0;

	std::map<int, int>::iterator it = listenerTimeoutMs.find(listenerFd);
	if (it != listenerTimeoutMs.end())
	{
		c.timeoutMs = it->second;
		if (c.timeoutMs > 0)
			c.expiresAtMs = nowMs() + (long long)c.timeoutMs;
	}
}

void resetClientTimeout(Client &c)
{
	if (c.timeoutMs > 0)
		c.expiresAtMs = nowMs() + (long long)c.timeoutMs;
}

int calcNextTimeout(std::map<int, Client> &clients, int fallbackMs)
{
	long long now = nowMs();
	long long earliest = 0;

	std::map<int, Client>::iterator it = clients.begin();
	while (it != clients.end())
	{
		Client &c = it->second;
		if (c.timeoutMs > 0 && c.expiresAtMs > 0)
		{
			if (earliest == 0)
				earliest = c.expiresAtMs;
			else if (c.expiresAtMs < earliest)
				earliest = c.expiresAtMs;
		}
		++it;
	}
	if (earliest == 0)
		return fallbackMs;
	long long delta = earliest - now;
	if (delta <= 0)
		return 0;
	if (delta > INT_MAX)
		return (INT_MAX);
	return (int)delta;
}

void closeExpiredClients(std::vector<struct pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clientList)
{
	if (clientList.empty())
		return;
	long long now = nowMs();
	std::vector<int> toClose;
	std::map<int, Client>::const_iterator it = clientList.begin();
	while (it != clientList.end())
	{
		const Client &c = it->second;
		if (c.timeoutMs > 0 && c.expiresAtMs > 0)
		{
			if (now >= c.expiresAtMs)
				toClose.push_back(it->first);
		}
		++it;
	}
	for (size_t i = 0; i < toClose.size(); ++i)
		closeClient(toClose[i], pollFdList, fdIndex, clientList);
}
