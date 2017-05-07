#ifndef STRIDEHTTPSERVER_H_INCLUDED
#define STRIDEHTTPSERVER_H_INCLUDED

#include <iostream>
#include <memory>
#include <unistd.h>

#include "Poco/Exception.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Util::ServerApplication;

using namespace std;

namespace stride {

class StrideRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	StrideRequestHandlerFactory();
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request);

private:
	// (Put our window into the simulator here)
};

class StrideServer
{
public:
	StrideServer();
	~StrideServer();

	// Run the server on the given port.
	// The server runs on the current thread until it is terminated.
	void start(unsigned short port);

	// Stop the server.
	void stop();

private:
	StrideRequestHandlerFactory* factory;
	unique_ptr<HTTPServer> server;
};

} // namespace Stride

#endif // end of include guard
