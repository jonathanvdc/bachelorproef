#include <iterator>
#include <sstream>

#include "Poco/JSON/Object.h"
#include "Poco/URI.h"

#include "handlers.h"
#include "server.h"

using namespace std;

namespace stride {

// StrideRequestHandlerFactory

StrideRequestHandlerFactory::StrideRequestHandlerFactory() {}

HTTPRequestHandler* StrideRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	Poco::URI uri(request.getURI());

	vector<string> path;
	uri.getPathSegments(path);

	vector<pair<string, string> > params = uri.getQueryParameters();
	map<string, string> paramMap(params.begin(), params.end());

	// Debug: Print the URL path to console
	cout << "Path: ";
	for (string& i : path) 
		cout << i << '/';
	cout << endl;

	// Debug: Print the Parameters to console
	cout << "Parameters: ";
	for (pair<string, string>& i : params)
		cout << i.first << '=' << i.second << ' ';
	cout << endl;

	if (path[0] == string("API")) {
		if (path.size() == 1)
			return new NotFoundRequestHandler();

		if (path[1] == "online")
			return new DataRequestHandler();
		else if (path[1] == "math")
			return new MathRequestHandler(paramMap);
	} else
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