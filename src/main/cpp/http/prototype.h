#ifndef STRIDEHTTPPROTO_H_INCLUDED
#define STRIDEHTTPPROTO_H_INCLUDED

#include <iostream>
#include <memory>
#include <unistd.h>

#include "Poco/Exception.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::ThreadPool;
using namespace std;

namespace stride {

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

class HelloWorldServer
{
public:
	HelloWorldServer();
	~HelloWorldServer();
	void start(unsigned short port);
	void stop();

private:
	unique_ptr<HTTPServer> server;
	HelloWorldRequestHandlerFactory* factory;
};

} // namespace Stride

#endif // include guard