#include "dlgaliases.h"
#include "ui_dlgaliases.h"
#include "QStandardItem"

dlgaliases::dlgaliases(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgaliases)
{
    ui->setupUi(this);
    QStandardItemModel *model = new QStandardItemModel(2, 2, this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Alias")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Command")));
}

dlgaliases::~dlgaliases()
{
    delete ui;
}
