#include <iostream>
#include "prototype.h"

using namespace std;
using namespace Stride;

// Adapted from Poco/Net/samples/HTTPTimeServer

// HelloWorldRequestHandler

HelloWorldRequestHandler::HelloWorldRequestHandler(const std::string& format) : _format(format) {}

void HelloWorldRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	Application& app = Application::instance();
	app.logger().information("Request from " + request.clientAddress().toString());

	Timestamp now;
	std::string dt(DateTimeFormatter::format(now, _format));

	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html");

	std::ostream& ostr = response.send();
	ostr << "Hello, World!";
}

// HelloWorldRequestHandlerFactory

HelloWorldRequestHandlerFactory::HelloWorldRequestHandlerFactory(const std::string& format) : _format(format) {}

HTTPRequestHandler* HelloWorldRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	if (request.getURI() == "/")
		return new HelloWorldRequestHandler(_format);
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

void HTTPHelloWorldServer::uninitialize() { ServerApplication::uninitialize(); }

void HTTPHelloWorldServer::defineOptions(OptionSet& options)
{
	ServerApplication::defineOptions(options);

	options.addOption(
	    Option("help", "h", "display argument help information")
		.required(false)
		.repeatable(false)
		.callback(OptionCallback<HTTPHelloWorldServer>(this, &HTTPHelloWorldServer::handleHelp)));
}

void HTTPHelloWorldServer::handleHelp(const std::string& name, const std::string& value)
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");
	helpFormatter.setHeader("A web server that posts \"Hello, world!\".");
	helpFormatter.format(std::cout);
	stopOptionsProcessing();
	_helpRequested = true;
}

int HTTPHelloWorldServer::main(const std::vector<std::string>& args)
{
	if (!_helpRequested) {
		unsigned short port = (unsigned short)config().getInt("HTTPHelloWorldServer.port", 9980);

		std::string format(config().getString("HTTPHelloWorldServer.format", DateTimeFormat::SORTABLE_FORMAT));

		ServerSocket svs(port);
		HelloWorldRequestHandlerFactory* factory = new HelloWorldRequestHandlerFactory(format);
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

void VizProto::run()
{
	char* argc[] = {"stride"};
	app.run(1, argc);
}

void VizProto::kill() { app.terminate(); }
