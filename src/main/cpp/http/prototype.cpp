#include "prototype.h"

using namespace std;
using namespace stride;

// Adapted from Poco/Net/samples/HTTPTimeServer

// HelloWorldRequestHandler

HelloWorldRequestHandler::HelloWorldRequestHandler() {}

void HelloWorldRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html");

	std::ostream& ostr = response.send();
	ostr << "Hello, World!";
}
// HelloWorldRequestHandlerFactory

HelloWorldRequestHandlerFactory::HelloWorldRequestHandlerFactory() {}

HTTPRequestHandler* HelloWorldRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	if (request.getURI() == "/")
		return new HelloWorldRequestHandler();
	return nullptr;
}

// HelloWorldServer

HelloWorldServer::HelloWorldServer() {}
HelloWorldServer::~HelloWorldServer() {}

void HelloWorldServer::start(unsigned short port)
{
	ServerSocket svs(port);
	factory = new HelloWorldRequestHandlerFactory();
	HTTPServerParams* params = new HTTPServerParams();
	server = make_unique<HTTPServer>(factory, svs, params);
	server->start();
}

void HelloWorldServer::stop() { server->stop(); }