#include "../../includes/server.hpp"

server::server() : _port(-1), _backlog(-1), _autoindex(false),
	_client_timeout_sec(-1), _max_body_size(-1),
	_body_ram_threshold(-1), _header_cap(-1) {}

server::~server() {}


