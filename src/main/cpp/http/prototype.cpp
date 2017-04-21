#include "prototype.h"
#include <iostream>

using namespace std;
using namespace Stride;


// HelloWorldRequestHandler

HelloWorldRequestHandler::HelloWorldRequestHandler(const std::string& format): _format(format) {}

void HelloWorldRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response){
    Application& app = Application::instance();
    app.logger().information("Request from "
        + request.clientAddress().toString());

    Timestamp now;
    std::string dt(DateTimeFormatter::format(now, _format));

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    std::ostream& ostr = response.send();
    ostr << "Hello, World!";
}


// HelloWorldRequestHandlerFactory

HelloWorldRequestHandlerFactory::HelloWorldRequestHandlerFactory(const std::string& format):
    _format(format)
{
}

HTTPRequestHandler* HelloWorldRequestHandlerFactory::createRequestHandler(
    const HTTPServerRequest& request)
{
    if (request.getURI() == "/")
        return new HelloWorldRequestHandler(_format);
    else
        return 0;
}


// HTTPHelloWorldServer

HTTPHelloWorldServer::HTTPHelloWorldServer(): _helpRequested(false) {}
HTTPHelloWorldServer::~HTTPHelloWorldServer() {}

void HTTPHelloWorldServer::initialize(Application& self)
{
    loadConfiguration();
    ServerApplication::initialize(self);
}

void HTTPHelloWorldServer::uninitialize()
{
    ServerApplication::uninitialize();
}

void HTTPHelloWorldServer::defineOptions(OptionSet& options)
{
    ServerApplication::defineOptions(options);

    options.addOption(
    Option("help", "h", "display argument help information")
        .required(false)
        .repeatable(false)
        .callback(OptionCallback<HTTPHelloWorldServer>(
            this, &HTTPHelloWorldServer::handleHelp)));
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
    if (!_helpRequested)
    {
        unsigned short port = (unsigned short) config().getInt("HTTPHelloWorldServer.port", 9980);

        std::string format(config().getString("HTTPHelloWorldServer.format", DateTimeFormat::SORTABLE_FORMAT));

        ServerSocket svs(port);
        HTTPServer srv(new HelloWorldRequestHandlerFactory(format), svs, new HTTPServerParams);
        srv.start();
        waitForTerminationRequest();
        srv.stop();
    }
    return Application::EXIT_OK;
}

// VizProto

void VizProto::run(){
    char** argc = new char*("stride");
    app.run(1, argc);
}

void VizProto::kill(){
    app.terminate();
}

