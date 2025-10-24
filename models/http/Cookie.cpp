#include "Cookie.hpp"

Cookie::Cookie() {}

Cookie::Cookie(const Cookie &other)
{
	(void)other;
}

Cookie &Cookie::operator=(const Cookie &other)
{
	(void)other;
	return *this;
}

Cookie::~Cookie() {}

std::string Cookie::createSessionID()
{
	const int sessionIdLength = 32;
	std::string sessionId;
	std::ifstream urandom("/dev/urandom", std::ios::binary);

	if (!urandom)
	{
		// Fallback: use time + random
		static bool seeded = false;
		if (!seeded)
		{
			std::srand(std::time(0) ^ clock());
			seeded = true;
		}

		const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		for (int i = 0; i < sessionIdLength; ++i)
		{
			sessionId += chars[std::rand() % chars.length()];
		}
	}
	else
	{
		// Use /dev/urandom for cryptographically secure random bytes
		unsigned char buffer[sessionIdLength / 2]; // Each byte becomes 2 hex chars
		urandom.read(reinterpret_cast<char *>(buffer), sessionIdLength / 2);

		std::ostringstream oss;
		for (size_t i = 0; i < sessionIdLength / 2; ++i)
		{
			oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
		}
		sessionId = oss.str();
		urandom.close();
	}
	sessionMetadata[sessionId];
	return sessionId;
}

void Cookie::addHashedFileToSession(const std::string &sessionId, std::string &fileHash)
{
	sessionMetadata[sessionId].push_back(fileHash);
}

void Cookie::removeHashedFileFromSession(const std::string &sessionId, std::string &fileHash)
{
    std::vector<std::string> &vec = sessionMetadata[sessionId];
    vec.erase(std::remove(vec.begin(), vec.end(), fileHash), vec.end());
}
