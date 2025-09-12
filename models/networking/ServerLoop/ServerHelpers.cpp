#include "ServerHelpers.hpp"
#include "Timeout.hpp"

void buildSimpleResponse(Client &c)
{
	// Build a small, styled HTML page
	std::string body;
	body += "<!doctype html>\n";
	body += "<html lang=\"en\">\n";
	body += "<head>\n";
	body += "  <meta charset=\"utf-8\">\n";
	body += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
	body += "  <title>Welcome to webserv</title>\n";
	body += "  <style>\n";
	body += "    html,body{height:100%;margin:0}\n";
	body += "    body{display:flex;align-items:center;justify-content:center;";
	body += "background:linear-gradient(135deg,#1f2937,#0ea5e9);";
	body += "font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Helvetica,Arial,sans-serif;";
	body += "color:#fff}\n";
	body += "    .card{background:rgba(255,255,255,0.1);padding:32px 40px;border-radius:16px;";
	body += "box-shadow:0 10px 30px rgba(0,0,0,0.25);backdrop-filter:blur(6px);text-align:center}\n";
	body += "    h1{margin:0 0 8px;font-size:32px}\n";
	body += "    p{margin:0;font-size:14px;opacity:.9}\n";
	body += "  </style>\n";
	body += "</head>\n";
	body += "<body>\n";
	body += "  <div class=\"card\">\n";
	body += "    <h1>Welcome to webserv</h1>\n";
	body += "    <p>Your server is up and running.</p>\n";
	body += "  </div>\n";
	body += "</body>\n";
	body += "</html>\n";

	std::ostringstream oss;
	oss << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: text/html; charset=utf-8\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "Connection: keep-alive\r\n" // closed to close connection after sending response, keep-alive to keep connection alive
		<< "\r\n"
		<< body;

	c.outBuff = oss.str();
	c.closeFlag = false; // close after sending this response
}

void closeClient(int fd, std::vector<pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clientList)
{
	delPollFd(pollFdList, fdIndex, fd);
	std::map<int, Client>::iterator itr = clientList.find(fd);
	if (itr != clientList.end())
		clientList.erase(itr);
	close(fd);
	std::cout << "closed " << fd << "\n";
}

bool headersComplete(const std::string &s)
{
	return (s.find("\r\n\r\n") != std::string::npos);
}

void readOnce(int fd, std::vector<struct pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clients)
{
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end())
		return;
	Client &c = it->second;
	char buf[4096];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);

	if (n > 0)
	{
		c.inBuff.append(buf, (size_t)n);
		resetClientTimeout(c);
		if (c.outBuff.empty() && headersComplete(c.inBuff))
		{

			// buildSimpleResponse(c);
			if (c.responseQueued == false)
			{
				HttpHandler::handleRequest(c.request, c.response, c.inBuff);
				c.outBuff = c.response.toString();
				// std::cout << c.outBuff << std::endl;
				c.responseQueued = true;
			}
			modifyEvent(pollFdList, fdIndex, fd, (short)(POLLIN | POLLOUT));
		}
		return;
	}
	closeClient(fd, pollFdList, fdIndex, clients);
}

void writeOnce(int fd, std::vector<struct pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clients)
{
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end())
		return;
	Client &c = it->second;
	if (c.outBuff.empty())
	{
		modifyEvent(pollFdList, fdIndex, fd, POLLIN);
		return;
	}
	// std::cout << "Http Response to be sent by client: " << c.fd << "\n" << c.outBuff << std::endl;
	ssize_t n = send(fd, c.outBuff.data(), c.outBuff.size(), 0);
	if (n > 0)
	{
		c.outBuff.erase(0, (size_t)n);
		resetClientTimeout(c);
		if (c.outBuff.empty())
		{
			modifyEvent(pollFdList, fdIndex, fd, POLLIN);
			if (c.closeFlag)
			{
				std::cout << "closing, 2nd loop with no content" << std::endl;
				closeClient(fd, pollFdList, fdIndex, clients);
			}
			else{
				c.inBuff.clear();
				c.responseQueued = false;
				c.closeFlag = false;
			}
		}
		return;
	}
	closeClient(fd, pollFdList, fdIndex, clients);
}

void acceptClients(int lfd, std::vector<struct pollfd> &pollFdList, std::map<int, size_t> &fdIndex, std::map<int, Client> &clientList, std::map<int, int> &listenerTimeoutMs)
{
	while (1)
	{
		int clientFd = accept(lfd, 0, 0);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			std::cerr << "accept error\n";
			break;
		}
		if (setNonBlocking(clientFd) == false)
		{
			close(clientFd);
			continue;
		}
		addPollFd(pollFdList, fdIndex, clientFd, POLLIN);
		Client c;
		c.fd = clientFd;
		setClientTimeout(lfd, c, listenerTimeoutMs);
		clientList[clientFd] = c;
		std::cout << "accepted " << clientFd << "\n";
	}
}
