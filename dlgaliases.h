#ifndef DLGALIASES_H
#define DLGALIASES_H

#include <QDialog>

namespace Ui {
class dlgaliases;
}

class dlgaliases : public QDialog
{
    Q_OBJECT
    
public:
    explicit dlgaliases(QWidget *parent = 0);
    ~dlgaliases();
    
private:
    Ui::dlgaliases *ui;
};

#endif // DLGALIASES_H
