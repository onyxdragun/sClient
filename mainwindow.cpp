#include "mainwindow.h"
#include "dlgconnect.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), iEnableKeepAlive(1), iMaxIdle(30),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->txtInput->installEventFilter(this);
    iNumKeepAlives = 2;
    iInterval = 2;
    iHistoryPos = 0;
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(slotConnect()));
    connect(ui->txtInput, SIGNAL(returnPressed()), this, SLOT(readInput()));

    connect(ui->actionFonts, SIGNAL(triggered()), this, SLOT(loadFontsDialog()));

    socket = new QTcpSocket(this);
    socket->setReadBufferSize(2048);

    fd = socket->socketDescriptor();
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &iEnableKeepAlive, sizeof(iEnableKeepAlive));

    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &iMaxIdle, sizeof(iMaxIdle));

    // send up to 3 keepalive packets out, then disconnect if no response
    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &iNumKeepAlives, sizeof(iNumKeepAlives));

    // send a keepalive packet out every 2 seconds (after the 5 second idle period)
    setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &iInterval, sizeof(iInterval));

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
    /* Add command to the beginging of the history */
    std::vector<QString>::iterator itHistory;
    itHistory = vHistory.begin();
    if (vHistory.size() >= HISTORY_MAX_SIZE) {
        vHistory.pop_back();
    }
    vHistory.insert(itHistory, sInput);

    if ( (sInput.indexOf(".") != -1) && (sInput.indexOf(".") == 0) && (sInput.length() > 1) )
    {
        if(ui->actionSpeedwalk->isChecked())
        {
            qDebug() << "Speedwalk requested";
            doSpeedWalk(sInput);
        }
    }
    else if (sInput.indexOf(";") != -1)
    {
        qDebug() << "Potential command stacking requested";
    }
    else
    {
        if(isConnected)
        {
            socket->write(sInput.toStdString().c_str());
            socket->write("\n");
        }
    }
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
    ui->txtOutput->appendPlainText("Connecting to Server...");
    socket->connectToHost(sHostAddress, iHostPort);
    if(!socket->waitForConnected())
    {
        ui->txtOutput->appendPlainText("Could not connect to server");
        qDebug() << socket->errorString();
    }
}

void MainWindow::connected()
{
    QString sStatus = this->sHostAddress +" "+ QString::number(this->iHostPort);
    ui->txtOutput->appendPlainText("Connected to Server");
    isConnected = true;
    this->setWindowTitle(sStatus);
}

void MainWindow::disconnected()
{
    ui->txtOutput->appendPlainText("Disconnected from Server");
    ui->txtOutput->appendPlainText(socket->errorString());
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
    while(socket->bytesAvailable())
    {
        qDebug() << "Length: " << socket->bytesAvailable();
        //ui->txtOutput->append(socket->readAll());
        data += QString::fromUtf8(socket->readAll());
        emit displayText(data);
    }
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

void MainWindow::displayText(QByteArray data)
{
    ui->txtOutput->appendPlainText(data);
    QTextCursor c = ui->txtOutput->textCursor();
    c.movePosition(QTextCursor::End);
    ui->txtOutput->setTextCursor(c);
}

void MainWindow::doSpeedWalk(QString sWalk)
{
    sWalk = sWalk.mid(1);
    std::istringstream iss(sWalk.toStdString());
    std::vector<std::pair<int, char> > vRoute;
    std::vector<std::pair<int, char> >::iterator vIt;
    qDebug() << "Path requested: " << sWalk;
    int num;
    char dir;
    if(isConnected) {
        while (iss >> num >> dir) {
            vRoute.push_back(std::make_pair(num, dir));
        }
        for(vIt = vRoute.begin(); vIt != vRoute.end(); vIt++)
        {
            qDebug() << "Walking " << vIt->first << " " << vIt->second;
            for(int i = 0; i < vIt->first; i++)
            {
                //qDebug() << vIt->second;
                std::stringstream ss;
                std::string sDir;
                ss << vIt->second;
                ss >> sDir;
                socket->write(sDir.c_str());
                socket->write("\n");
            }
        }

    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent *event)
{
    if (obj == ui->txtInput)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Up)
            {
                /* Check if there is history and show the command */
                if (iHistoryPos < vHistory.size() && iHistoryPos < HISTORY_MAX_SIZE) {
                    showHistoryItem(iHistoryPos);
                    iHistoryPos++;
                }
            }
            else if (keyEvent->key() == Qt::Key_Down)
            {
                if (iHistoryPos > 0) {
                    showHistoryItem(iHistoryPos);
                    iHistoryPos--;
                }
                else if (iHistoryPos == 0)
                {
                    showHistoryItem(iHistoryPos);
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::showHistoryItem(int pos)
{
    if (pos >= 0 && pos < vHistory.size())
    {
        ui->txtInput->setText(vHistory[pos]);
        ui->txtInput->selectAll();
    }
}
