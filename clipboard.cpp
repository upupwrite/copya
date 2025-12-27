#include "clipboard.h"
#include <iostream>
#include <chrono>
#include <cstdio>

Monitor::Monitor(QObject* parent)
    : QObject(parent), shouldStop(false), isMonitoring(false) {
    std::cout << "Clipboard Monitor initialized. Use start() to begin monitoring." << std::endl;
}

Monitor::~Monitor() {
    // 确保线程被正确停止
    stop();

    // 等待线程结束
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}

void Monitor::start() {
    if (isMonitoring) {
        std::cout << "Monitor is already running!" << std::endl;
        return;
    }

    shouldStop = false;
    isMonitoring = true;

    // 创建并启动监控线程
    monitorThread = std::thread(&Monitor::monitoringLoop, this);

    std::cout << "Monitoring clipboard for text changes (running in background)..." << std::endl;
    std::cout << "Use stop() to end monitoring." << std::endl;
}

void Monitor::stop() {
    if (!isMonitoring) {
        return;
    }

    std::cout << "Stopping clipboard monitor..." << std::endl;
    shouldStop = true;
    isMonitoring = false;

    // 等待线程结束
    if (monitorThread.joinable()) {
        monitorThread.join();
    }

    std::cout << "Clipboard monitor stopped." << std::endl;
}

bool Monitor::isRunning() {
    return isMonitoring.load();
}

void Monitor::monitoringLoop() {
    std::string previous_text;
    int empty_count = 0;

    while (!shouldStop.load()) {
        // 读取当前剪贴板内容
        FILE* pipe = popen("wl-paste --type text 2>/dev/null", "r");
        if (!pipe) {
            std::cerr << "Failed to execute wl-paste!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        char buffer[1024];
        std::string current_text;

        // 读取所有输出
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            current_text += buffer;
        }

        pclose(pipe);

        // 移除末尾换行符
        while (!current_text.empty() &&
               (current_text.back() == '\n' || current_text.back() == '\r')) {
            current_text.pop_back();
        }

        // 检查是否变化
        if (current_text != previous_text) {
            if (!current_text.empty()) {
                std::cout << "\n[Clipboard] Text changed:" << std::endl;
                std::cout << "----------------------------------------" << std::endl;
                std::cout << current_text << std::endl;
                db.writeText(current_text);
                std::cout << "Saved to database" << std::endl;
                std::cout << "----------------------------------------" << std::endl;
                previous_text = current_text;
                empty_count = 0;
                emit clipTextChanged();
            } else if (previous_text != "" && ++empty_count <= 1) {
                // 剪贴板变空时也记录一次
                std::cout << "\n[Clipboard] Cleared" << std::endl;
                previous_text = "";
                emit clipTextChanged();
            }
        }

        // 调整轮询间隔
        if (!current_text.empty()) {
            // 有内容时检查频繁一些
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } else {
            // 空内容时检查间隔长一些
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

bool setClipboardWithWlCopy(const std::string& text) {
    // 使用 wl-copy 设置剪贴板内容
    FILE* pipe = popen("wl-copy", "w");
    if (!pipe) {
        std::cerr << "Failed to execute wl-copy!" << std::endl;
        return false;
    }

    // 写入文本到 wl-copy 的标准输入
    size_t written = fwrite(text.c_str(), 1, text.length(), pipe);
    fclose(pipe);

    if (written != text.length()) {
        std::cerr << "Failed to write all text to clipboard!" << std::endl;
        return false;
    }

    return true;
}
