#ifndef F_SONDE_H
#define F_SONDE_H

#include <QWidget>
#include "Arduino.h"
#include <QStandardItem>

namespace Ui {
class F_Sonde;
}

class F_Sonde :public QWidget
{
    Q_OBJECT

public:
    explicit F_Sonde(Arduino *oMonArduino, QWidget *parent = 0);
    ~F_Sonde();

private slots:
    void on_btnModeAcquisition_clicked();

    //void on_cbModeVisualisation_toggled(bool checked);

    void on_btnLancer_clicked();

    void on_btnEnregistrement_clicked(bool checked);

private:
    Ui::F_Sonde *ui;
    Arduino *oArduino;
    QStandardItemModel *model = new QStandardItemModel;

};

#endif // F_SONDE_H
