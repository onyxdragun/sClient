#include "dlgconnect.h"
#include "ui_dlgconnect.h"

dlgConnect::dlgConnect(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::dlgConnect)
{
    ui->setupUi(this);
    mainWindow = parent;

    connect(ui->bConnect, SIGNAL(clicked()), this, SLOT(doConnect()));
}

dlgConnect::~dlgConnect()
{
    delete ui;
}

// doConnect()
//
// Read values entered by user and store them in MainWindow for usage
void dlgConnect::doConnect()
{
    mainWindow->setHost(ui->txtServerAddr->text());
    mainWindow->setPort(ui->txtPort->text().toInt());
    qDebug() << "Attempting server connection...";
    mainWindow->doConnection();
    this->close();
}
