#include <iterator>
#include <sstream>

#include "Poco/JSON/Object.h"
#include "Poco/URI.h"

#include "handlers.h"
#include "server.h"

using namespace std;

namespace stride {

// StrideRequestHandlerFactory

StrideRequestHandlerFactory::StrideRequestHandlerFactory(
    shared_ptr<multiregion::SimulationTask<StrideSimulatorResult>> task)
    : task(task)
{
}

HTTPRequestHandler* StrideRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	Poco::URI uri(request.getURI());

	// Get the path as a vector of segments
	vector<string> path;
	uri.getPathSegments(path);

	// Get the parameters as a map of names to values
	vector<pair<string, string>> params = uri.getQueryParameters();
	map<string, string> paramMap(params.begin(), params.end());

	// Debug: Print the URL path to console
	// cout << "Path: ";
	// for (string& i : path)
	// 	cout << i << '/';
	// cout << endl;

	// Debug: Print the Parameters to console
	// cout << "Parameters: ";
	// for (pair<string, string>& i : params)
	// 	cout << i.first << '=' << i.second << ' ';
	// cout << endl;

	// Determine our response by breaking down the path
	if (path[0] == string("API")) {
		if (path.size() == 1)
			return new ErrorRequestHandler("Path not found!");

		if (path[1] == "online")
			return new OnlineRequestHandler();
		else if (path[1] == "math")
			return new MathRequestHandler(paramMap);
		else if (path[1] == "sim") {
			if (path.size() == 2)
				return new ErrorRequestHandler("Path not found!");

			if(path[2] == "infectedCount")
				return new InfectedCountRequestHandler(task);

			// if(path[2] == "currentDay")
			// 	return new NotFoundRequestHandler();
		}
	}
	return new ErrorRequestHandler("Path not found!");
}

// StrideServer

StrideServer::StrideServer(vector<shared_ptr<multiregion::SimulationTask<StrideSimulatorResult>>> myTasks)
{
	task = myTasks[0];
	cout << task->GetPopulationSize() << endl;
	cout << task->GetInfectedCount() << endl;
}

StrideServer::~StrideServer() {}

void StrideServer::start(unsigned short port)
{
	ServerSocket svs(port);
	factory = new StrideRequestHandlerFactory(task);
	HTTPServerParams* params = new HTTPServerParams;
	server = make_shared<HTTPServer>(factory, svs, params);
	server->start();
}

void StrideServer::stop() { server->stop(); }

} // namespace stride