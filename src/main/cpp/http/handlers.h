#ifndef STRIDEHTTPHANDLERS_H_INCLUDED
#define STRIDEHTTPHANDLERS_H_INCLUDED

#include <iostream>
#include <unistd.h>
#include <memory>

#include "sim/run_stride.h"
#include "multiregion/ParallelSimulationManager.h"
#include "multiregion/SimulationManager.h"

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

using namespace std;

namespace stride {

class ErrorRequestHandler : public HTTPRequestHandler
{
public:
	ErrorRequestHandler(string message);
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
private:
	string message;
};

class OnlineRequestHandler : public HTTPRequestHandler
{
public:
	OnlineRequestHandler();
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};

class InfectedCountRequestHandler : public HTTPRequestHandler
{
public:
	InfectedCountRequestHandler(shared_ptr<multiregion::SimulationTask<StrideSimulatorResult>> task);
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
private:
	size_t count;
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