#include "VhostHelpers.hpp"

static void replaceLowerCase(std::string& s) {
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned char uc = static_cast<unsigned char>(s[i]);
		int lc = std::tolower(uc);
		s[i] = static_cast<char>(lc);
	}
}

static void trimWhiteSpace(std::string& s) {
	const char* ws = " \t";
	std::string::size_type begin = s.find_first_not_of(ws);
	if (begin == std::string::npos){
		s.clear();
		return;
	}
	std::string::size_type end = s.find_last_not_of(ws);
	s = s.substr(begin, end - begin + 1);
	}

static bool lowerCaseHeaders(const std::string& rawHeaders, const char* headerName, std::string& out) {
	std::string lower = rawHeaders; // lower case raw header
		replaceLowerCase(lower);
	std::string toFind = headerName; // lower case host name
		replaceLowerCase(toFind);
	toFind += ":";
	std::string::size_type pos = lower.find(toFind); // look for Host:, and get the value behind
	if (pos == std::string::npos)
		return false;
	pos += toFind.size();
	while (pos < rawHeaders.size() && (rawHeaders[pos] == ' ' || rawHeaders[pos] == '\t'))
		++pos;

	std::string::size_type eol = rawHeaders.find("\r\n", pos);
	if (eol == std::string::npos)
		eol = rawHeaders.size();
	out = rawHeaders.substr(pos, eol - pos);
	trimWhiteSpace(out);
	return true;
}

static bool getHostValue(const std::string& rawHeaders, std::string& outHost) {
	std::string host;
	if (!lowerCaseHeaders(rawHeaders, "Host", host)) //get host name
		return false;

	replaceLowerCase(host);
	std::string::size_type colon = host.find(':');
	if (colon != std::string::npos) {
		host = host.substr(0, colon);
	}
	trimWhiteSpace(host);
	outHost = host; // return host name 
	return true;
}

const Server* ServerLoop::vhostPicker_(const std::vector<const Server*>& candidates,
	const std::string& rawHeaders)
{
	std::string hostLowerNoPort;
	bool haveHost = getHostValue(rawHeaders, hostLowerNoPort);

	if (haveHost) {
		for (size_t i = 0; i < candidates.size(); ++i) {
			const Server* srv = candidates[i];
			for (size_t j = 0; j < srv->name.size(); ++j) {
				std::string candidate = srv->name[j];
				replaceLowerCase(candidate);
				trimWhiteSpace(candidate);
				if (candidate == hostLowerNoPort) //try to match if candidate name is = to whatever we are looking for in the request
					return srv;
			}
		}
	}
	if (!candidates.empty())
		return candidates[0];
	return NULL;
}
