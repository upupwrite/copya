#include <QAction>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>  // 添加 QDebug 头文件
#include <QDir>
#include <QLibraryInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTranslator>
#include <iostream>

#include "copya.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    QApplication::setApplicationName("Copya");
    QApplication::setApplicationVersion("1.1.2");
    QApplication::setOrganizationName("Copya");

    // 安装 Qt 自身翻译
    QTranslator qtTranslator;
    // 使用新的方法获取翻译路径
    QString qtTranslationsPath =
        QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (qtTranslator.load(QLocale::system(), "qt", "_", qtTranslationsPath))
    {
        a.installTranslator(&qtTranslator);
    }

    // 安装应用程序翻译
    QTranslator appTranslator;
    QString locale = QLocale::system().name();  // 如: "zh_CN"

    // 尝试从多个位置加载翻译文件
    QStringList searchPaths = {
        QApplication::applicationDirPath() + "/translations", ":/translations",
        QDir::currentPath() + "/translations"};

    bool loaded = false;
    for (const QString &path : searchPaths)
    {
        qDebug() << "Looking for translation in:" << path;
        if (appTranslator.load(QString("Copya_%1").arg(locale),
                               path))  // 修改为 Copya
        {
            loaded = true;
            qDebug() << "Translation loaded from:" << path;
            break;
        }
    }

    if (loaded)
    {
        a.installTranslator(&appTranslator);
    }
    else
    {
        qWarning() << "Translation file not found for locale:" << locale;
    }

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
        // 已经有实例在运行
        if (parser.isSet(menuOption))
        {
            // 如果有 --menu 参数，发送 toggle 消息
            socket.write("toggle");
            socket.waitForBytesWritten(1000);
        }
        else
        {
            // 如果没有参数，激活现有窗口
            socket.write("activate");
            socket.waitForBytesWritten(1000);
        }
        socket.close();
        return 0;  // 直接退出，不启动新实例
    }
    else
    {
        isFirstInstance = true;
    }
    socket.close();

    // 创建本地服务器用于进程间通信
    QLocalServer server;
    if (!server.listen(serverName))
    {
        // 如果监听失败，先移除可能存在的旧服务器
        QLocalServer::removeServer(serverName);
        if (!server.listen(serverName))
        {
            qWarning() << "Cannot start single instance server";
            return -1;
        }
    }

    // 创建主窗口但不立即显示
    Copya w;

    // 第一次启动时，如果有 --menu 参数，立即显示窗口
    if (isFirstInstance && parser.isSet(menuOption))
    {
        w.show();
    }
    else if (!parser.isSet(menuOption))
    {
        // 第一次启动且没有 --menu 参数，默认隐藏窗口
        w.hide();
    }

    // 连接服务器信号
    QObject::connect(
        &server, &QLocalServer::newConnection, &w,
        [&server, &w]()
        {
            QLocalSocket *clientSocket = server.nextPendingConnection();
            if (!clientSocket)
                return;

            QObject::connect(
                clientSocket, &QLocalSocket::readyRead,
                [&w, clientSocket]()
                {
                    QByteArray data = clientSocket->readAll();
                    qDebug() << "Received message from other instance:" << data;

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

    // 创建系统托盘图标
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&w);

    // 使用 Qt 内置图标（可以替换为自己的图标）
    QIcon appIcon(":/main/icon/copya.png");  // 建议使用自己的图标
    if (appIcon.isNull())
    {
        trayIcon->setIcon(
            QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    }
    else
    {
        trayIcon->setIcon(appIcon);
    }

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
    QObject::connect(quitAction, &QAction::triggered,
                     [&a, &server, serverName]()
                     {
                         server.close();
                         QLocalServer::removeServer(serverName);
                         a.quit();
                     });

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
