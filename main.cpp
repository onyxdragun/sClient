#include <QtWidgets/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;

    QObject::connect(&app, SIGNAL(aboutToQuit()), &window, SLOT(shuttingDown()));
    window.show();
    
    return app.exec();
}
