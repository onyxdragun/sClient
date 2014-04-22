#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTcpSocket>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setHost(QString sHost);
    void setPort(int iPort);
    void setStatusBar();
    void doConnection();
    
private:
    Ui::MainWindow *ui;
    QString sHostAddress;
    int iHostPort;
    QLabel *statusLabel;
    QTcpSocket *socket;
    bool isConnected;


public slots:
    void readInput();
    void slotConnect();
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

};

#endif // MAINWINDOW_H
