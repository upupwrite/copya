#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QStyle>
#include <QSystemTrayIcon>

#include "copya.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 创建主窗口但不显示
    Copya w;

    // 创建系统托盘图标
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&w);

    // 使用Qt内置图标（选择一种）
    // 方法1：使用QStyle的标准图标
    trayIcon->setIcon(
        QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    // 或者使用其他内置图标：
    // SP_MessageBoxInformation 信息图标
    // SP_FileIcon 文件图标
    // SP_DirIcon 文件夹图标
    // SP_DriveHDIcon 硬盘图标

    // 方法2：使用QIcon的fromTheme（如果系统有图标主题）
    // trayIcon->setIcon(QIcon::fromTheme("edit-copy")); // 复制图标

    // 创建托盘菜单
    QMenu *trayMenu = new QMenu();
    QAction *showAction = trayMenu->addAction("显示窗口");
    QAction *hideAction = trayMenu->addAction("隐藏窗口");
    trayMenu->addSeparator();
    QAction *quitAction = trayMenu->addAction("退出");

    trayIcon->setContextMenu(trayMenu);

    // 连接信号槽
    QObject::connect(showAction, &QAction::triggered, &w, &Copya::show);
    QObject::connect(hideAction, &QAction::triggered, &w, &Copya::hide);
    QObject::connect(quitAction, &QAction::triggered, &a, &QApplication::quit);

    // 托盘图标点击事件
    QObject::connect(trayIcon, &QSystemTrayIcon::activated,
                     [&w](QSystemTrayIcon::ActivationReason reason)
                     {
                         if (reason == QSystemTrayIcon::DoubleClick ||
                             reason == QSystemTrayIcon::Trigger)
                         {
                             if (w.isVisible())
                             {
                                 w.hide();
                             }
                             else
                             {
                                 w.show();
                                 w.activateWindow();
                                 w.raise();
                             }
                         }
                     });

    trayIcon->show();
    trayIcon->showMessage("Copya", "程序已启动，可通过托盘图标控制",
                          QSystemTrayIcon::Information, 3000);

    return a.exec();
}
