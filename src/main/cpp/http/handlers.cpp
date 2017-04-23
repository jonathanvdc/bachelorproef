
#include "handlers.h"

namespace Stride{
// NotFoundRequestHandler

NotFoundRequestHandler::NotFoundRequestHandler() {}

void NotFoundRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.set("Access-Control-Allow-Origin", request.get("Origin"));
	response.setChunkedTransferEncoding(true);
	response.setContentType("json");

	Poco::JSON::Object out;
	out.set("message", "Not found!");

	std::ostream& ostr = response.send();
	out.stringify(ostr);
}
}