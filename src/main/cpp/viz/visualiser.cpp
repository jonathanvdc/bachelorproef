/*
    This is a prototype for the Awesomium web view,
    adapted from an example included in the SDK
*/

#include "visualiser.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

using namespace std;

namespace Stride {

Visualiser::Visualiser(){}

void Visualiser::run(){
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QtWebEngine::initialize();

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/viz.qml")));

    return app.exec();
}

}
