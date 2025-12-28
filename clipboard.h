#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "database.h"

class Monitor : public QObject
{
    Q_OBJECT

public:
    Monitor(QObject *parent = nullptr);
    ~Monitor();

    void start();      // 开始监控
    void stop();       // 停止监控
    bool isRunning();  // 检查是否在运行

signals:
    void clipTextChanged();

private:
    void monitoringLoop();  // 监控循环的私有方法

    std::thread monitorThread;       // 监控线程
    std::atomic<bool> shouldStop;    // 停止标志
    std::mutex clipboardMutex;       // 互斥锁
    std::atomic<bool> isMonitoring;  // 监控状态

    ClipDatabase db;  // 数据库实例
};

bool setClipboardWithWlCopy(const std::string &text);

#endif  // CLIPBOARD_H
