#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <RoiData.h>
#include <MainViewWindow.h>
#include <AppLogger.h>
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    AppLogger::install();
  qmlRegisterType<CameraFrameItem>("AppItems", 1, 0, "CameraFrameItem");
    qmlRegisterUncreatableType<RoiData>("MyApp", 1, 0, "RoiData", "Created by RoiManager only");
     MianViewModel mVM;
    mVM.Initialize();
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("mainViewModel",&mVM);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("QT456TDirect", "Main");

    return app.exec();
}
