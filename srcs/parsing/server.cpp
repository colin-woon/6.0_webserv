#include "../../includes/server.hpp"

server::server() : port(-1), backlog(-1), autoindex(false),
	client_timeout_sec(-1), max_body_size(-1),
	body_ram_threshold(-1), header_cap(-1) {}

server::~server() {}


