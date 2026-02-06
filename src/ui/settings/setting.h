#ifndef SETTING_H
#define SETTING_H

#include <QDialog>

namespace Ui
{
class Setting;
}

class Setting : public QDialog
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = nullptr);
    ~Setting();

private:
    QWidget *select = findChild<QWidget *>("select");
    QWidget *display = findChild<QWidget *>("display");
    QWidget *conform = findChild<QWidget *>("conform");

    Ui::Setting *ui;
};

#endif  // SETTING_H
