#include "Cookies.hpp"

Cookies::Cookies() {}

Cookies::Cookies(const Cookies &other)
{
	(void)other;
}

Cookies &Cookies::operator=(const Cookies &other)
{
	(void)other;
	return *this;
}

Cookies::~Cookies() {}

std::string Cookies::createSessionID()
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
