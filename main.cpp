#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "copya.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 创建主窗口但不显示
    Copya w;

    // 创建系统托盘图标
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&w);
    trayIcon->setIcon(QIcon(":/icons/app_icon.png")); // 设置图标

    // 创建托盘菜单
    QMenu *trayMenu = new QMenu();
    QAction *showAction = trayMenu->addAction("显示窗口");
    QAction *hideAction = trayMenu->addAction("隐藏窗口");
    trayMenu->addSeparator();
    QAction *quitAction = trayMenu->addAction("退出");

    trayIcon->setContextMenu(trayMenu);

    // 修正连接语句
    QObject::connect(showAction, &QAction::triggered, &w, &Copya::show);
    QObject::connect(hideAction, &QAction::triggered, &w, &Copya::hide);
    QObject::connect(quitAction, &QAction::triggered, &a, &QApplication::quit);

    // 托盘图标点击事件（使用lambda表达式）
    QObject::connect(trayIcon, &QSystemTrayIcon::activated, [&w](QSystemTrayIcon::ActivationReason reason){
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
            if (w.isVisible()) {
                w.hide();
            } else {
                w.show();
                w.activateWindow();
                w.raise();
            }
        }
    });

    trayIcon->show();
    trayIcon->showMessage("Copya", "程序已启动，可通过托盘图标控制", QSystemTrayIcon::Information, 3000);

    return a.exec();
}
