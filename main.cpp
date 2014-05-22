#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(shuttingDown()));
    w.show();
    
    return a.exec();
}
