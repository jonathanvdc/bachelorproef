#include "Poco/JSON/Object.h"
#include "server.h"

using namespace std;

namespace Stride {

// NotFoundRequestHandler

NotFoundRequestHandler::NotFoundRequestHandler() {}

void NotFoundRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setChunkedTransferEncoding(true);
	response.setContentType("json");

	Poco::JSON::Object out;
	out.set("message", "Not found!");

	std::ostream& ostr = response.send();
	out.stringify(ostr);
}

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