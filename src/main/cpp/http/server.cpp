#include "Poco/JSON/Object.h"
#include "server.h"
#include "handlers.h"

using namespace std;

namespace Stride {

// StrideRequestHandlerFactory

StrideRequestHandlerFactory::StrideRequestHandlerFactory() {}

HTTPRequestHandler* StrideRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	string url = request.getURI();
	if (url == "/API/")
		return new NotFoundRequestHandler();
	else
		return nullptr;
}

// StrideServer

StrideServer::StrideServer() {}
StrideServer::~StrideServer() {}

int StrideServer::run(unsigned short port)
{
	ServerSocket svs(port);
	StrideRequestHandlerFactory* factory = new StrideRequestHandlerFactory();
	HTTPServerParams* params = new HTTPServerParams;
	HTTPServer srv(factory, svs, params);
	// Start the server
	srv.start();
	// Wait until some call for termination occurs
	waitForTerminationRequest();
	// Stop the server
	srv.stop();
	delete factory;
	return Application::EXIT_OK;
}
}