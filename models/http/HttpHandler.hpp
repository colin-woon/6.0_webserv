#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpExceptions.hpp"
#include "HttpRequestParser.hpp"
#include "HttpHandlerGET.hpp"
#include "HttpHandlerPOST.hpp"
#include "HttpHandlerDELETE.hpp"
#include "Router.hpp"
#include "../networking/ServerLoop/ServerLoop.hpp"
#include "../parsing/Server.hpp"
#include "../parsing/Location.hpp"

class HttpHandler : public HttpHandlerGET, public HttpHandlerPOST, public HttpHandlerDELETE
{
public:
	static void handleRequest(Client &client);
};
#endif
