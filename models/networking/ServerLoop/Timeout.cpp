#include "Timeout.hpp"

uint64_t nowMs()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	uint64_t ms = (uint64_t)tv.tv_sec * 1000;
	ms += (uint64_t)(tv.tv_usec / 1000);
	return (ms);
}

void buildTimeoutList(std::vector<int> &listenerFdList, std::vector<Server> &serverList, std::map<int, int> &listenerTimeoutMs) //build map for listenerfd -> timeout
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
			uint64_t tmp = (uint64_t)serverList[i].client_timeout_sec * 1000;
			if (tmp > INT_MAX)
				ms = INT_MAX;
			else
				ms = (int)tmp;
		}
		listenerTimeoutMs[listenerFdList[i]] = ms;  // store in map
	}
}

void setClientTimeout(int listenerFd, Client &c, std::map<int, int> &listenerTimeoutMs) //set timeout on client based on server settings
{
	c.timeoutMs = 0; // reset 
	c.expiresAtMs = 0;

	std::map<int, int>::iterator it = listenerTimeoutMs.find(listenerFd); // find listenerFd time timeout list, add current time with timeout time
	if (it != listenerTimeoutMs.end())
	{
		c.timeoutMs = it->second;
		if (c.timeoutMs > 0)
			c.expiresAtMs = nowMs() + (uint64_t)c.timeoutMs;
	}
}

void resetClientTimeout(Client &c)
{
	if (c.timeoutMs > 0)
		c.expiresAtMs = nowMs() + (uint64_t)c.timeoutMs;
}

int calcNextTimeout(std::map<int, Client> &clients, std::map<int, CGIcontext>& CGImap, int fallbackMs)
{
	uint64_t now = nowMs();
	uint64_t earliest = 0;

	std::map<int, Client>::iterator it = clients.begin();
	while (it != clients.end())
	{
		Client &c = it->second;
		if (c.timeoutMs > 0 && c.expiresAtMs > 0) // go through all client, find earliest to expire
		{
			if (earliest == 0)
				earliest = c.expiresAtMs;
			else if (c.expiresAtMs < earliest)
				earliest = c.expiresAtMs;
		}
		// for (std::map<int, std::map<int, CGIcontext>::iterator>::iterator ctxIt = c.CGIiter.begin();
		// 		ctxIt != c.CGIiter.end(); ctxIt++)
		// {
		// 	CGIcontext& ctx = ctxIt->second->second;
		// 	if (ctx.timeoutMs > 0 && ctx.expiresAtMs > 0
		// 		&& ctx.expiresAtMs < earliest)
		// 			earliest = ctx.expiresAtMs;
		// }
		++it;
	}
	for (std::map<int, CGIcontext>::iterator ctx = CGImap.begin();
			ctx != CGImap.end(); ctx++)
	{
		if (ctx->second.timeoutMs > 0 && ctx->second.expiresAtMs > 0
			&& ctx->second.expiresAtMs < earliest)
				earliest = ctx->second.expiresAtMs;
	}
	if (earliest == 0)
		return fallbackMs;
	uint64_t delta = earliest - now;
	if (delta <= 0) // already expire
		return 0;
	if (delta > INT_MAX) // debug for delta too big
		return (INT_MAX);
	return (int)delta; // return next timeout time
}
