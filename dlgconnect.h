#ifndef DLGCONNECT_H
#define DLGCONNECT_H

#include <QDialog>
#include <mainwindow.h>

namespace Ui {
class dlgConnect;
}

class dlgConnect : public QDialog
{
    Q_OBJECT
    
public:
    explicit dlgConnect(MainWindow *parent = 0);
    ~dlgConnect();
    
private:
    Ui::dlgConnect *ui;
    MainWindow *mainWindow;

private slots:
    void doConnect();

};

#endif // DLGCONNECT_H
