#ifndef STRIDEHTTPPROTO_H_INCLUDED
#define STRIDEHTTPPROTO_H_INCLUDED
#include <iostream>
#include <unistd.h>

#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Process.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;

namespace Stride {

class HelloWorldRequestHandler : public HTTPRequestHandler
{
public:
	HelloWorldRequestHandler();
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};

class HelloWorldRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	HelloWorldRequestHandlerFactory();
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request);
};

class HTTPHelloWorldServer : public Poco::Util::ServerApplication
{
public:
	HTTPHelloWorldServer();
	~HTTPHelloWorldServer();
	int run(unsigned short port);
};

class VizProto
{
public:
	VizProto();
	void run(unsigned short port);
	void kill();

private:
	HTTPHelloWorldServer app;
};
}
#endif // include guard