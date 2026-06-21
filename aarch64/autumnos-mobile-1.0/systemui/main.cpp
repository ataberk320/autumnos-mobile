#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QCursor>
#include <QScreen>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
	
    app.setOverrideCursor(Qt::ArrowCursor);
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
    	screen->setPhysicalSize(QSize(480, 800)); 
    }    
    
    QQmlApplicationEngine engine;    
    engine.load(QUrl(QStringLiteral("qrc:/UntitledProject/App.qml")));
    
    return app.exec();
}
