#include <QAction>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QSystemTrayIcon>

#include "copya.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    QApplication::setApplicationName("Copya");
    QApplication::setApplicationVersion("1.1.2");
    QApplication::setOrganizationName("Copya");

    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("Copya - 剪贴板管理工具");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption menuOption(QStringList() << "m" << "menu",
                                  "显示或隐藏应用程序窗口");
    parser.addOption(menuOption);

    parser.process(a);

    // 单实例检测 - 使用唯一名称
    QString serverName = "copya_app" + QDir::homePath().replace("/", "_");
    QLocalSocket socket;
    socket.connectToServer(serverName);

    // 如果已经有实例运行
    bool isFirstInstance = false;
    if (socket.waitForConnected(500))
    {
        // 处理--menu参数
        if (parser.isSet(menuOption))
        {
            socket.write("toggle");
        }
        else
        {
            socket.write("activate");
        }
        socket.waitForBytesWritten(1000);
        socket.close();

        // 无论是否有--menu参数，都不启动新实例，直接退出
        return 0;
    }
    else
    {
        isFirstInstance = true;
    }
    socket.close();

    // 如果没有已有实例，正常启动程序

    // 创建本地服务器用于进程间通信
    QLocalServer server;
    if (!server.listen(serverName))
    {
        QLocalServer::removeServer(serverName);
        if (!server.listen(serverName))
        {
            qWarning() << "Cannot start single instance server";
            return -1;
        }
    }

    // 创建主窗口但不立即显示
    Copya w;
    if (isFirstInstance)
    {
        w.init();
    }

    // 连接服务器信号
    QObject::connect(
        &server, &QLocalServer::newConnection, &w,
        [&server, &w]()
        {
            QLocalSocket *clientSocket = server.nextPendingConnection();
            if (!clientSocket)
                return;

            QObject::connect(clientSocket, &QLocalSocket::readyRead,
                             [&w, clientSocket]()
                             {
                                 QByteArray data = clientSocket->readAll();
                                 if (data == "toggle")
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
                                 else if (data == "activate")
                                 {
                                     w.show();
                                     w.activateWindow();
                                     w.raise();
                                 }
                                 clientSocket->close();
                                 clientSocket->deleteLater();
                             });
        });

    // 如果使用--menu参数启动，第一次启动时显示窗口
    // 如果没有--menu参数，窗口保持隐藏（只显示托盘）
    if (parser.isSet(menuOption))
    {
        w.show();
    }

    // 创建系统托盘图标
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&w);

    // 使用Qt内置图标
    trayIcon->setIcon(
        QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));

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
