#include <iostream>
#include <iterator>
#include <thread>
#include <gtest/gtest.h>

#include "http/prototype.h"

#include <Poco/Net/HTTPCredentials.h>
#include "Poco/Exception.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/NetException.h"
#include "Poco/NullStream.h"
#include "Poco/Path.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"

using namespace std;
using namespace stride;

// Poco
using Poco::Net::HTTPClientSession;
using Poco::Net::ConnectionRefusedException;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::StreamCopier;
using Poco::Path;
using Poco::URI;
using Poco::Exception;

string doRequest(
    Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response)
{
	session.sendRequest(request);
	istream& rs = session.receiveResponse(response);

	if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
		throw Poco::IOException();

	// return the answer as a string
	return string(istreambuf_iterator<char>(rs), {});
}

bool queryServer(unsigned short port)
{
	this_thread::sleep_for(2s);
	cout << "Querying server." << endl;

	URI uri("http://127.0.0.1");
	string path(uri.getPathAndQuery());
	if (path.empty())
		path = "/";

	try {
		HTTPClientSession session(uri.getHost(), port);
		HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
		HTTPResponse response;
		try {
			string answer = doRequest(session, request, response);
			cout << "Received answer \"" << answer << "\"" << endl;
			return true;
		} catch (Poco::IOException e) {
			cout << "Connection denied!" << endl;
			return false;
		}
	} catch (ConnectionRefusedException e) {
		cout << "Connection refused!" << endl;
		return false;
	}
}

namespace Tests {

TEST(HTTP, ServerPrototype)
{
	auto s = make_shared<HelloWorldServer>();
	s->start(1234);
	EXPECT_TRUE(queryServer(1234));
	s->stop();
}

} // Tests
