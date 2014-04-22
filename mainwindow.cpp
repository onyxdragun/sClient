#include "mainwindow.h"
#include "dlgconnect.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(slotConnect()));
    connect(ui->txtInput, SIGNAL(returnPressed()), this, SLOT(readInput()));

    connect(ui->actionFonts, SIGNAL(triggered()), this, SLOT(loadFontsDialog()));

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()),this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyRead()));

    statusLabel = new QLabel();

    isConnected = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotConnect()
{
    dlgConnect *dConnect = new dlgConnect(this);
    dConnect->show();
}

void MainWindow::readInput()
{
    QString sInput = ui->txtInput->text();
    if(isConnected)
    {
        socket->write(sInput.toStdString().c_str());
        socket->write("\n");
    }
    //ui->txtOutput->appendPlainText(sInput);
    ui->txtInput->clear();
}

void MainWindow::setHost(QString sHost)
{
    this->sHostAddress = sHost;
}

void MainWindow::setPort(int iPort)
{
    this->iHostPort = iPort;
}

void MainWindow::setStatusBar()
{
    QString sStatus = "Server: "+ this->sHostAddress +" Port: "+ QString::number(this->iHostPort);
    statusLabel->setText(sStatus);
    ui->statusBar->addWidget(statusLabel);
}

void MainWindow::doConnection()
{
    ui->txtOutput->append("Connecting to Server...");
    socket->connectToHost(sHostAddress, iHostPort);
    if(!socket->waitForConnected())
    {
        ui->txtOutput->append("Could not connect to server");
    }
}

void MainWindow::connected()
{
    QString sStatus = this->sHostAddress +" "+ QString::number(this->iHostPort);
    ui->txtOutput->append("Connected to Server");
    isConnected = true;
    this->setWindowTitle(sStatus);
}

void MainWindow::disconnected()
{
    ui->txtOutput->append("Disconnected to Server");
    isConnected = false;
    this->setWindowTitle("Not Connected");
    statusLabel->clear();
}

void MainWindow::bytesWritten(qint64 bytes)
{
    qDebug() << bytes << " bytes written...";
}

void MainWindow::readyRead()
{
    qDebug() << "Reading from Socket...";

    // read the data from the socket
    ui->txtOutput->append(socket->readAll());
    QTextCursor c = ui->txtOutput->textCursor();
    c.movePosition(QTextCursor::End);
    ui->txtOutput->setTextCursor(c);
}

void MainWindow::loadFontsDialog()
{
    bool ok = true;
    QFont font = QFontDialog::getFont(&ok, this);
    if(!ok)
    {
        return;
    }
    ui->txtOutput->setFont(font);
}
