#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTcpSocket>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QPalette>

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
    QByteArray data;


public slots:
    void readInput();
    void slotConnect();
    void loadFontsDialog();
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
    void displayText(QByteArray data);

};

#endif // MAINWINDOW_H
