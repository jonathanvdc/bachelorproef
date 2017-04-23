#include <iostream>
#include "prototype.h"

using namespace std;
using namespace Stride;

// Adapted from Poco/Net/samples/HTTPTimeServer

// HelloWorldRequestHandler

HelloWorldRequestHandler::HelloWorldRequestHandler() {}

void HelloWorldRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	Application& app = Application::instance();
	app.logger().information("Request from " + request.clientAddress().toString());

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
	else
		return nullptr;
}

// HTTPHelloWorldServer

HTTPHelloWorldServer::HTTPHelloWorldServer() : _helpRequested(false) {}
HTTPHelloWorldServer::~HTTPHelloWorldServer() {}

void HTTPHelloWorldServer::initialize(Application& self)
{
	loadConfiguration();
	ServerApplication::initialize(self);
}

int HTTPHelloWorldServer::run(unsigned short port)
{
	if (!_helpRequested) {
		ServerSocket svs(port);
		HelloWorldRequestHandlerFactory* factory = new HelloWorldRequestHandlerFactory();
		HTTPServerParams* params = new HTTPServerParams;
		HTTPServer srv(factory, svs, params);
		srv.start();
		waitForTerminationRequest();
		srv.stop();
		delete factory;
	}
	return Application::EXIT_OK;
}

// VizProto

VizProto::VizProto() {}

void VizProto::run(unsigned short port) { app.run(port); }

void VizProto::kill() { app.terminate(); }
