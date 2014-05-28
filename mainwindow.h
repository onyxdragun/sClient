#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTcpSocket>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QPalette>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <QMap>
#include <QSettings>

#define HISTORY_MAX_SIZE 50

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
    void doSpeedWalk(QString sWalk);
    void doStackedCommands(QString sInput);
    
private:
    Ui::MainWindow *ui;
    QString sHostAddress;
    int iHostPort;
    int iEnableKeepAlive;
    int fd;
    int iMaxIdle;
    int iNumKeepAlives;
    int iInterval;
    QLabel *statusLabel;
    QTcpSocket *socket;
    bool isConnected;
    std::vector<QString> vHistory;
    size_t iHistoryPos;
    void showHistoryItem(size_t);
    QMap<QString,QString> mAliases;
    bool addToAliases(QString);

    void SaveSettings();
    void LoadSettings();
    void LoadAliases(QSettings*);
    void SaveAliases(QSettings*);
    void ProcessInput(QString);
    void PrintAliases();

    void displayText(QByteArray data);

public slots:
    void readInput();
    void slotConnect();
    void slotAliases();
    void loadFontsDialog();
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
    //void displayText(QByteArray data);
    void shuttingDown();

protected:
    bool eventFilter(QObject* obj, QEvent *event);

};

#endif // MAINWINDOW_H
