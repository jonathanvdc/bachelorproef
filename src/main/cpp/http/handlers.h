#ifndef STRIDEHTTPHANDLERS_H_INCLUDED
#define STRIDEHTTPHANDLERS_H_INCLUDED

#include <iostream>
#include <unistd.h>
#include "Poco/Exception.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

namespace stride {

class NotFoundRequestHandler : public HTTPRequestHandler
{
public:
	NotFoundRequestHandler();
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};

class DataRequestHandler : public HTTPRequestHandler
{
public:
	DataRequestHandler();
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};

class MathRequestHandler : public HTTPRequestHandler
{
public:
	MathRequestHandler(std::map<std::string, std::string> paramMap);
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

private:
	double x, y;
	std::string op;
};

} // namespace Stride

#endif // end of include guard