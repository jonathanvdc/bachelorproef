#ifndef STRIDEHTTPHANDLERS_H_INCLUDED
#define STRIDEHTTPHANDLERS_H_INCLUDED

#include <iostream>
#include <unistd.h>
#include "Poco/Exception.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JSON/Object.h"

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

namespace Stride {

class NotFoundRequestHandler : public HTTPRequestHandler
{
public:
	NotFoundRequestHandler();
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};
}

#endif // end of include guard