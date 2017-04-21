#include <iostream>
#include <gtest/gtest.h>
#include <thread>
#include <iterator>

#include "viz/prototype.h"
#include "pop/Population.h"
#include "sim/Simulator.h"

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include <Poco/Net/HTTPCredentials.h>
#include "Poco/StreamCopier.h"
#include "Poco/NullStream.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"

using namespace std;
using namespace Stride;

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


void runServer(VizProto* p){
    cout << "Running server!" << endl;
    p->run();
}

string doRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response){
    session.sendRequest(request);
    istream& rs = session.receiveResponse(response);
    
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED){
        // return the answer as a string
        return string(istreambuf_iterator<char>(rs), {});
    }else{
        //it went wrong...?
        return string("");
    }
}

bool queryServer(VizProto* p){
    this_thread::sleep_for(2s);
    cout << "Querying server." << endl;

    URI uri("http://127.0.0.1:9980/");
    string path(uri.getPathAndQuery());
    if(path.empty()) path = "/";

    try {
        HTTPClientSession session(uri.getHost(), uri.getPort());
        HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        HTTPResponse response;
        string answer = doRequest(session, request, response);
        if(answer != string("")){
            cout << "Received answer \"" << answer << "\"" << endl;
            return true;
        } else {
            cout << "Received no answer...?" << endl;
            return false;
        }
    } catch(ConnectionRefusedException e){
        cout << "Connection refused!" << endl;
        return false;
    }

    // Kill the server thread, causes the entire test to crash without any warning, so don't...
    // p->kill();
}

namespace Tests {

// Tests that make use of X (the linux window system) need to end in '_x'
TEST(Visualiser, VisualiserRunPrototype)
{
    VizProto* p = new VizProto();
    thread serverThread(runServer, p);
    // thread clientThread(queryServer, p);

    EXPECT_TRUE(queryServer(p));

    //...detach the thread, since I can find no good way of killing it humanely
    // results in a few seconds waiting after the tests complete running...
    // TODO: figure out how to kill this thing
    serverThread.detach();
    return;
}

} // Tests
