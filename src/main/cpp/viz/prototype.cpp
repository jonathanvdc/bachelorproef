/*
    This is a prototype for the Awesomium web view,
    adapted from an example included in the SDK
*/

#include "prototype.h"

using namespace Awesomium;
using namespace std;

namespace Stride {

void AwesomiumProto::main(){

    // Create the WebCore singleton with default configuration
    WebCore* web_core = WebCore::Initialize(WebConfig());

    // Create a new WebView instance with a certain width and height, using the
    // WebCore we just created
    WebView* view = web_core->CreateWebView(800, 600);

    // Load a certain URL into our WebView instance
    WebURL url(WSLit("https://github.com/flu-plus-plus"));
    view->LoadURL(url);
    cout << "Page is now loading..." << endl;

    // Wait for our WebView to finish loading
    while (view->IsLoading())
        Update(50);

    cout << "Page has finished loading." << endl;

    cout << "Page title is: " << view->title() << endl;

    // Update once more a little longer to allow scripts and plugins
    // to finish loading on the page.
    Update(300);

    // Get the WebView's rendering Surface. The default Surface is of
    // type 'BitmapSurface', we must cast it before we can use it.
    BitmapSurface* surface = (BitmapSurface*)view->surface();

    // Make sure our surface is not NULL-- it may be NULL if the WebView 
    // process has crashed.
    if (surface != NULL) {
        // Save our BitmapSurface to a JPEG image
        surface->SaveToJPEG(WSLit("./result.jpg"));

        cout << "Saved a render of the page to 'result.jpg'." << endl;
    }

    // Destroy our WebView instance
    view->Destroy();

    // Update once more before we shutdown for good measure
    Update(100);

    // Destroy our WebCore instance
    WebCore::Shutdown();

    return;
}

void AwesomiumProto::Update(int sleep_ms) {
    // Sleep a specified amount
    usleep(sleep_ms * 1000);

    // You must call WebCore::update periodically
    // during the lifetime of your application.
    WebCore::instance()->Update();
}

}