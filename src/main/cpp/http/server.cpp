#include <iterator>
#include <sstream>

#include "Poco/JSON/Object.h"

#include "handlers.h"
#include "server.h"

using namespace std;

namespace Stride {

vector<string> split(const string& s, char delim)
{
	stringstream ss;
	ss.str(s);
	string item;
	vector<string> items;
	while (getline(ss, item, delim))
		items.push_back(item);
	return items;
}

// StrideRequestHandlerFactory

StrideRequestHandlerFactory::StrideRequestHandlerFactory() {}

HTTPRequestHandler* StrideRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	vector<string> url = split(request.getURI(), '/');
	for(string& i : url)
		cout << i << '/';
	cout << endl;
	if (url[1] == string("API"))
	{
		if (url.size() > 2 && url[2] == "Gimme")
			return new DataRequestHandler();
		return new NotFoundRequestHandler();
	}
	else
		return new NotFoundRequestHandler();
}

// StrideServer

StrideServer::StrideServer() {}
StrideServer::~StrideServer() {}

int StrideServer::run(unsigned short port)
{
	ServerSocket svs(port);
	StrideRequestHandlerFactory* factory = new StrideRequestHandlerFactory();
	HTTPServerParams* params = new HTTPServerParams;
	HTTPServer srv(factory, svs, params);
	// Start the server
	srv.start();
	// Wait until some call for termination occurs
	waitForTerminationRequest();
	// Stop the server
	srv.stop();
	delete factory;
	return Application::EXIT_OK;
}
}