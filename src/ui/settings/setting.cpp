#include "setting.h"

#include <qradiobutton.h>

#include <QButtonGroup>
#include <QDebug>

#include "ui_setting.h"

Setting::Setting(QWidget *parent) : QDialog(parent), ui(new Ui::Setting)
{
    ui->setupUi(this);
    // 创建按钮组
    QButtonGroup *buttonGroup = new QButtonGroup(this);

    // 添加 radioButton 到组中

    buttonGroup->addButton(select->findChild<QRadioButton *>("radioButton"), 1);
    buttonGroup->addButton(select->findChild<QRadioButton *>("radioButton_2"),
                           2);
    buttonGroup->addButton(select->findChild<QRadioButton *>("radioButton_3"),
                           3);
    buttonGroup->addButton(select->findChild<QRadioButton *>("radioButton_4"),
                           4);

    connect(buttonGroup,
            QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this,
            [buttonGroup](QAbstractButton *button)
            {
                if (button)
                {
                    int id = buttonGroup->id(button);
                    QString text = button->text();
                    qDebug() << "选中按钮ID:" << id << "文本:" << text;
                }
            });
}

Setting::~Setting() { delete ui; }
