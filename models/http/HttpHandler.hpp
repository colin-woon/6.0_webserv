#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpExceptions.hpp"
#include "HttpUtils.hpp"
#include "HttpHandlerGET.hpp"
#include "../networking/ServerLoop/ServerLoop.hpp"

class HttpHandler : public HttpHandlerGET
{
public:
	static void handleRequest(Client &client);
};
#endif
