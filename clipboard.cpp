#include "clipboard.h"

#include <chrono>
#include <cstdio>
#include <iostream>

Monitor::Monitor(QObject *parent)
    : QObject(parent), shouldStop(false), isMonitoring(false), database(nullptr)
{
    std::cout
        << "Clipboard Monitor initialized. Use start() to begin monitoring."
        << std::endl;
    database = new ClipDatabase();
}

Monitor::~Monitor()
{
    // 确保线程被正确停止
    stop();

    // 等待线程结束
    if (monitorThread.joinable())
    {
        monitorThread.join();
    }
}

void Monitor::start()
{
    if (isMonitoring)
    {
        std::cout << "Monitor is already running!" << std::endl;
        return;
    }

    shouldStop = false;
    isMonitoring = true;

    // 创建并启动监控线程
    monitorThread = std::thread(&Monitor::monitoringLoop, this);

    std::cout
        << "Monitoring clipboard for text changes (running in background)..."
        << std::endl;
    std::cout << "Use stop() to end monitoring." << std::endl;
}

void Monitor::stop()
{
    if (!isMonitoring)
    {
        return;
    }

    std::cout << "Stopping clipboard monitor..." << std::endl;
    shouldStop = true;
    isMonitoring = false;

    // 等待线程结束
    if (monitorThread.joinable())
    {
        monitorThread.join();
    }

    std::cout << "Clipboard monitor stopped." << std::endl;
}

bool Monitor::isRunning() { return isMonitoring.load(); }

void Monitor::monitoringLoop()
{
    std::string previous_text = database->readLatestText();
    int empty_count = 0;
    bool was_sleeping = false;  // 标记是否刚从休眠恢复

    while (!shouldStop.load())
    {
        // 读取当前剪贴板内容
        FILE *pipe = popen("wl-paste --type text 2>/dev/null", "r");
        if (!pipe)
        {
            std::cerr << "Failed to execute wl-paste!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        char buffer[1024];
        std::string current_text;

        // 读取所有输出
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            current_text += buffer;
        }

        int status = pclose(pipe);

        // 检查命令是否成功执行
        bool command_failed = false;
        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0)
            {
                // wl-paste 可能因系统休眠而失败
                command_failed = true;
                was_sleeping = true;
            }
        }
        else
        {
            command_failed = true;
            was_sleeping = true;
        }

        // 移除末尾换行符
        while (!current_text.empty() &&
               (current_text.back() == '\n' || current_text.back() == '\r'))
        {
            current_text.pop_back();
        }

        // 如果命令执行失败，认为是休眠状态，跳过本次检查
        if (command_failed)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        // 检查是否刚从休眠恢复
        if (was_sleeping)
        {
            // 休眠恢复后，重新获取当前剪贴板内容
            previous_text = current_text;
            was_sleeping = false;

            // 休眠恢复后，如果剪贴板不为空且与数据库最新记录不同，则保存
            if (!current_text.empty() &&
                current_text != database->readLatestText())
            {
                std::cout << "\n[Clipboard] Resumed from sleep - Recording "
                             "current content:"
                          << std::endl;
                std::cout << "----------------------------------------"
                          << std::endl;
                std::cout << current_text << std::endl;
                database->writeText(current_text);
                std::cout << "Saved to database after sleep" << std::endl;
                std::cout << "----------------------------------------"
                          << std::endl;
                previous_text = current_text;
                empty_count = 0;
                emit clipTextChanged();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        // 检查是否变化
        if (current_text != previous_text)
        {
            if (!current_text.empty())
            {
                std::cout << "\n[Clipboard] Text changed:" << std::endl;
                std::cout << "----------------------------------------"
                          << std::endl;
                std::cout << current_text << std::endl;
                database->writeText(current_text);
                std::cout << "Saved to database" << std::endl;
                std::cout << "----------------------------------------"
                          << std::endl;
                previous_text = current_text;
                empty_count = 0;
                emit clipTextChanged();
            }
            else if (!previous_text.empty() && ++empty_count <= 1)
            {
                // 剪贴板变空时也记录一次
                std::cout << "\n[Clipboard] Cleared" << std::endl;
                previous_text = "";
                emit clipTextChanged();
            }
        }

        // 调整轮询间隔
        if (!current_text.empty())
        {
            // 有内容时检查频繁一些
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else
        {
            // 空内容时检查间隔长一些
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

bool setClipboardWithWlCopy(const std::string &text)
{
    // 使用 wl-copy 设置剪贴板内容
    FILE *pipe = popen("wl-copy", "w");
    if (!pipe)
    {
        std::cerr << "Failed to execute wl-copy!" << std::endl;
        return false;
    }

    size_t written = fwrite(text.c_str(), 1, text.length(), pipe);

    int status = pclose(pipe);

    if (written != text.length())
    {
        std::cerr << "Failed to write all text to clipboard!" << std::endl;
        return false;
    }

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0)
        {
            std::cerr << "wl-copy exited with error code: " << exit_code
                      << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "wl-copy did not exit normally!" << std::endl;
        return false;
    }

    return true;
}
