#include "handlers.h"

using namespace std;

namespace stride {

// NotFoundRequestHandler

NotFoundRequestHandler::NotFoundRequestHandler() {}

void NotFoundRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.set("Access-Control-Allow-Origin", request.get("Origin"));
	response.setChunkedTransferEncoding(true);
	response.setContentType("json");

	Poco::JSON::Object out;
	out.set("responseType", "notFound");
	out.set("message", "Not found!");

	std::ostream& ostr = response.send();
	out.stringify(ostr);
}

// DataRequestHandler

DataRequestHandler::DataRequestHandler() {}

void DataRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.set("Access-Control-Allow-Origin", request.get("Origin"));
	response.setChunkedTransferEncoding(true);
	response.setContentType("json");

	Poco::JSON::Object out;
	out.set("responseType", "data");
	out.set("data", "this is the data");

	std::ostream& ostr = response.send();
	out.stringify(ostr);
}

// MathRequestHandler

MathRequestHandler::MathRequestHandler(map<string, string> paramMap)
{
	x = atof(paramMap["x"].c_str());
	y = atof(paramMap["y"].c_str());
	op = paramMap["op"];
	if (op == "")
		op = "add";
}

void MathRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.set("Access-Control-Allow-Origin", request.get("Origin"));
	response.setChunkedTransferEncoding(true);
	response.setContentType("json");

	double value;
	if (op == "add")
		value = x + y;
	else if (op == "mul")
		value = x * y;

	Poco::JSON::Object out;
	out.set("responseType", "value");
	out.set("value", value);

	std::ostream& ostr = response.send();
	out.stringify(ostr);
}

} // end-of-namespace