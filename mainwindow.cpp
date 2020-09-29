#include <sstream>

#include "mainwindow.h"
#include "dlgconnect.h"
#include "dlgaliases.h"

#include "ui_mainwindow.h"
#include "util.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), iEnableKeepAlive(1), iMaxIdle(30),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->txtInput->installEventFilter(this);
    ui->txtOutput->installEventFilter(this);

    iNumKeepAlives = 2;
    iInterval = 2;
    iHistoryPos = 0;

    LoadSettings();

    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(slotConnect()));
    connect(ui->actionAliases, SIGNAL(triggered()), this, SLOT(slotAliases()));
    connect(ui->txtInput, SIGNAL(returnPressed()), this, SLOT(readInput()));

    connect(ui->actionFonts, SIGNAL(triggered()), this, SLOT(loadFontsDialog()));

    socket = new QTcpSocket(this);

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

void MainWindow::slotAliases()
{
    dlgaliases *dAliases = new dlgaliases(this);
    dAliases->show();
}

// ProcessInput()
//
// QString [IN] - Read incoming string from user
//
// Based on what the user inputs, we will choose an action

void MainWindow::ProcessInput(QString sInput)
{
    size_t aaFound, swFound, stackFound = 0;
    size_t inputLength = 0;
    QTextCursor prevCursor = ui->txtOutput->textCursor();

    qDebug() << sInput;

    aaFound = sInput.toStdString().find("aa");
    swFound = sInput.toStdString().find(".");
    stackFound = sInput.toStdString().find(";");

    inputLength = sInput.length();

    // Speedwalk command issued
    if (swFound == 0 && inputLength > 1)
    {
        if (ui->actionSpeedwalk->isChecked())
        {
            qDebug() << "Speedwalk requested";
            doSpeedWalk(sInput);
        }
    }
    // User wishes to create an alias
    else if (aaFound == 0)
    {
        if (inputLength > 3)
        {
            addToAliases(sInput);
        }
        else
        {
            PrintAliases();
        }
    }
    // User is stacking commands
    else if (stackFound != std::string::npos)
    {
        qDebug() << "Stacked command found";
        doStackedCommands(sInput);
    }
    else
    {
        if (isConnected)
        {
            ui->txtOutput->moveCursor(QTextCursor::End);
            ui->txtOutput->textCursor().insertHtml("<span style=\"color:#ff0\">" + sInput + "</span><br style=\"color:#fff;\"/>");
            ui->txtOutput->moveCursor(QTextCursor::End);
            socket->write(sInput.toStdString().c_str());
            socket->write("\n");
        }
    }
}

// readInput()
//
// Read in what the user is typing into the textbox
void MainWindow::readInput()
{
    QString sInput = ui->txtInput->text();
    size_t found = 0;

    // Add command to the beginging of the history
    std::vector<QString>::iterator itHistory;
    iHistoryPos = 0;
    itHistory = vHistory.begin();
    if (vHistory.size() >= HISTORY_MAX_SIZE) {
        vHistory.pop_back();
    }
    if (sInput.compare(QString("")) != 0)
    {
        vHistory.insert(itHistory, sInput);
    }

    found = sInput.toStdString().find(" ");
    if (found == std::string::npos)
    {
        QMap<QString,QString>::const_iterator i = mAliases.find(sInput);
        // Check if it was an alias that was entered 
        if (i != mAliases.end())
        {
            qDebug() << "Alias \"" << i.key() << "\" called";
            ProcessInput(i.value());
        }
        else
        {
            ProcessInput(sInput);
        }
    }
    else {
        ProcessInput(sInput);
    }
    ui->txtInput->clear();
}

// setHost()
//
// QString [IN] - Set the IP of the server to connect to
void MainWindow::setHost(QString sHost)
{
    this->sHostAddress = sHost;
}

// setPort()
//
// QString [IN] - Set the port number of the server to connect to
void MainWindow::setPort(int iPort)
{
    this->iHostPort = iPort;
}

// doConnection()
//
// Attempt to connect to remote server using sHostAddress and iHostPost
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

// connected()
// 
// If connected set a flag that can be checked later
// Certain actions can only be done when connected
void MainWindow::connected()
{
    QString sStatus = this->sHostAddress +" "+ QString::number(this->iHostPort);
    ui->txtOutput->appendPlainText("Connected to Server");
    isConnected = true;
    this->setWindowTitle(sStatus);
}

// disconnected()
//
// Inform user if they have been disconnected from remote server
// Certain actions cannot be done if disconnected
void MainWindow::disconnected()
{
    ui->txtOutput->appendPlainText("Disconnected from Server");
    ui->txtOutput->appendPlainText(socket->errorString());
    isConnected = false;
    this->setWindowTitle("Not Connected");
    statusLabel->clear();
}

// bytesWritten()
//
// Debug - Check to see if bytes have been written to socket
void MainWindow::bytesWritten(qint64 bytes)
{
    qDebug() << "Bytes written: " << bytes;
}

// readyRead()
//
// Check the open socket and read from it
// Store all data into a QByteArray to process
void MainWindow::readyRead()
{
    // read the data from the socket
    while(socket->bytesAvailable())
    {
        qDebug() << "Bytes to read: " << socket->bytesAvailable();
        QByteArray data = socket->readAll();
        displayText(data);
    }
}

// loadFontsDialog()
// 
// All user to change Font preferences
void MainWindow::loadFontsDialog()
{
    bool ok = true;
    QFont font = QFontDialog::getFont(&ok, this);
    if (!ok)
    {
        return;
    }
    ui->txtOutput->setFont(font);
}

// displayText()
//
// QByteArray [IN] - Data read from socket
//
// Read the data that was received from the socket (remote server)
// Process it as needed and display to user
void MainWindow::displayText(QByteArray data)
{
    size_t tellFound = 0, shoutFound = 0;
    std::string str = QString(data).toStdString();
    QString txt;
    //QTextCursor prevCursor = ui->txtOutput->textCursor();

    tellFound = str.find(" tells you: ");
    shoutFound = str.find(" shouts: ");

    // ANSI colour codes could be present
    txt = util::processANSI(str);

    if (tellFound != std::string::npos)
    {
        // Shout or Tell was seen
        // Need to colour text to highlight as needed
        qDebug () << QString(data);
        txt = util::highlightStr(str, util::highlightTypes::TELL);
    }
    if (shoutFound != std::string::npos)
    {
        // Shout or Tell was seen
        // Need to colour text to highlight as needed
        qDebug () << QString(data);
        txt = util::highlightStr(str, util::highlightTypes::SHOUT);
    }

    ui->txtOutput->moveCursor(QTextCursor::End);
    ui->txtOutput->textCursor().insertHtml(txt);
    ui->txtOutput->moveCursor(QTextCursor::End);
    QTextCursor c = ui->txtOutput->textCursor();
    c.movePosition(QTextCursor::End);
    ui->txtOutput->setTextCursor(c);
}

// doSpeedWalk()
//
// QString [IN] - Speed walk directions
//
// Iterate through the string and pull out compass directions
// so that user can move multiple times with one command
void MainWindow::doSpeedWalk(QString sWalk)
{
    std::string speedWalk = sWalk.mid(1).toStdString(); // remove leading .
    std::vector<std::pair<int, char> > vRoute;
    std::vector<std::pair<int, char> >::iterator vIt;
    std::stringstream ss;
    int num = 1;

    qDebug() << "A walking path has been requested: " << sWalk;
    if (isConnected)
    {
        for (char const &c: speedWalk)
        {
            if (std::isdigit(c))
            {
                ss << c;
            }
            else
            {
                // Map the direction: 12 n, 9 e, 1 s
                std::istringstream(ss.str()) >> num;
                vRoute.push_back(std::make_pair(num, c));
                ss.clear();
                ss.str(std::string());
                num = 1;
            }
        }
        for (vIt = vRoute.begin(); vIt != vRoute.end(); ++vIt)
        {
            qDebug() << "Walking " << vIt->first << " " << vIt->second;
            for (int i = 0; i < vIt->first; i++)
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

// eventFilter()
//
// QObject [IN] - UI Object that triggered the event
// QEvent  [IN] - What event was triggered
bool MainWindow::eventFilter(QObject* obj, QEvent *event)
{
    if (obj == ui->txtInput)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            // Certain keyboard keys were issued
            switch (keyEvent->key())
            {
                case Qt::Key_Up:
                    // Check if there is history and show the command
                    if (iHistoryPos <= vHistory.size() && iHistoryPos <= HISTORY_MAX_SIZE)
                    {
                        showHistoryItem(iHistoryPos);
                        if ( (iHistoryPos+1) != vHistory.size())
                        {
                            iHistoryPos++;
                        }
                    }
                    break;
                case Qt::Key_Down:
                    // Iterate through history backwards
                    if (iHistoryPos > 0)
                    {
                        iHistoryPos--;
                        showHistoryItem(iHistoryPos);
                    }
                    else if (iHistoryPos == 0)
                    {
                        showHistoryItem(iHistoryPos);
                    }

                    break;
                default:
                    // Key strokes we do not care about
                    keyEvent->ignore();
                    break;
            }
        }
    }
    else if (obj == ui->txtOutput)
    {
        // We want to ensure that the txtInput receives the user input
        // and not the txtOutput.
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            ui->txtInput->insert(keyEvent->text());
            ui->txtInput->setFocus();
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// showHistoryItem()
//
// size_t [IN] - Index of item to display to user from Command History
void MainWindow::showHistoryItem(size_t pos)
{
    if (pos >= 0 && pos < vHistory.size())
    {
        ui->txtInput->setText(vHistory[pos]);
        ui->txtInput->selectAll();
    }
}

// doStackedCommands()
//
// QString [IN] - User input
//
// Iterate through user input and break up commands which are between
// ';' characters.
void MainWindow::doStackedCommands(QString sInput)
{
    std::vector<std::string> vStackedCmds;
    std::vector<std::string>::iterator vIt;
    std::string delimiter = ";";
    std::string str = sInput.toStdString();
    std::string token;
    size_t pos = 0;

    if (!ui->actionCmdStacking->isChecked())
    {
        return;
    }
    if (!isConnected)
    {
        return;
    }

    qDebug() << "Stacking Commands: " << sInput;

    // We need to split the string which is ; delimited 
    while ((pos = str.find(delimiter)) != std::string::npos)
    {
        token = str.substr(0, pos);
        vStackedCmds.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    // need to tack on last portion of the string 
    vStackedCmds.push_back(str);
    for (vIt = vStackedCmds.begin(); vIt != vStackedCmds.end(); vIt++)
    {
        //qDebug() << "Stacked Command " << vIt->c_str();
        socket->write((*vIt).c_str());
        socket->write("\n");
    }

}

// addToAliases()
//
// QString [IN] - String from user input
//
// User wishes to create alias. Aliases can do multiple
// actions with user only entering one word
//
bool MainWindow::addToAliases(QString input)
{
    bool b = true;
    std::string cmd, first, second;
    std::istringstream iss(input.toStdString());
    QString alias, command;
    iss >> cmd >> first;
    getline(iss, second);

    alias = QString(first.c_str());
    command = QString(second.c_str());
    qDebug() << "Command size: " << command.length();

    if (command.compare("\n") && (command.length() == 0))
    {
        // User wishes to remove alias (no action specified)
        mAliases.remove(alias);
        qDebug() << "Alias '" << alias << "' removed";
        ui->txtOutput->appendPlainText("## Removed sClient Alias: " + alias);
    }
    else
    {
        mAliases.insert(alias, command.trimmed());
        qDebug() << "Alias '" << alias << "' added";
        ui->txtOutput->appendPlainText("## Added sClient Alias: " + alias);
    }

    return b;
}

// SaveSettings()
//
// Save Program settings for later retrieval
// Currently Supported items:
//    Aliases
void MainWindow::SaveSettings()
{
    QSettings settings("DynamicShark", "sClient");
    SaveAliases(&settings);
}

// LoadSettings()
//
// Load Program settings for later retrieval
// Currently Supported items:
//    Aliases
void MainWindow::LoadSettings()
{
    QSettings settings("DynamicShark", "sClient");
    LoadAliases(&settings);
}

// SaveAliases()
//
// QString [IN] - Program settings
//
// Save user Aliases into storage
void MainWindow::SaveAliases(QSettings *settings)
{
    QMap<QString,QString>::iterator It = mAliases.begin();

    qDebug() << "Saving Aliases [" << mAliases.size() << "]...";
    if (mAliases.size() == 0)
    {
        settings->remove("Aliases");
    }
    else
    {
        settings->beginGroup("Aliases");
        while (It != mAliases.end())
        {
            qDebug() << "Alias: " << It.key() << " " << It.value();
            settings->setValue(It.key(), It.value());
            ++It;
        }
        settings->endGroup();
    }
    qDebug() << "Finished saving Aliases";
}

// LoadAliases()
//
// QString [IN] - Program settings
//
// Retrieve user Aliases from storage
void MainWindow::LoadAliases(QSettings *settings)
{
    settings->beginGroup("Aliases");
    QStringList keys = settings->childKeys();

    qDebug() << "Loading Aliases [" << keys.size() << "]...";
    foreach (QString key, keys)
    {
        QString cmd = settings->value(key).toString();
        mAliases.insert(key, cmd);
    }
    settings->endGroup();
}

// PrintAliases()
//
// Display the user Aliases that have been created
void MainWindow::PrintAliases()
{
    QMap<QString,QString>::const_iterator it = mAliases.begin();
    ui->txtOutput->appendPlainText("\n  sClient Aliases \t| Command");
    ui->txtOutput->appendHtml("<span style=\"color:#0f0\">+------------------------------------------------------------------+</span>");
    while (it != mAliases.end())
    {
        ui->txtOutput->appendPlainText("| " + it.key() + " \t\t| " + it.value());
        ++it;
    }
    ui->txtOutput->appendHtml("<span style=\"color:#0f0\">+------------------------------------------------------------------+</span><br />");
}

void MainWindow::shuttingDown()
{
    qDebug() << "Shutting down...";
    SaveSettings();
}
