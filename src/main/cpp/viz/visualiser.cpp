#include "visualiser.h"

using namespace std;

namespace Stride {

void Visualiser::run(){
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QtWebEngine::initialize();

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/viz.qml")));

    app.exec();
}

} // namespace Stride
