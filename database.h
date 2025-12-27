#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <tuple>
#include <sqlite3.h>

class ClipDatabase {
private:
    sqlite3* db;
    std::string dbPath;

    // 私有静态方法
    static bool createDirectoryIfNotExists(const std::string& path);
    static std::string expandHomePath(const std::string& path);

    // 私有成员方法
    void logError(const std::string& operation, const std::string& errorMsg) const;
    void logSuccess(const std::string& operation) const;

public:
    // 构造函数和析构函数
    explicit ClipDatabase(const std::string& databaseName = "~/.copya/user.db");
    ~ClipDatabase();

    // 禁用拷贝构造和赋值
    ClipDatabase(const ClipDatabase&) = delete;
    ClipDatabase& operator=(const ClipDatabase&) = delete;

    // 移动语义
    ClipDatabase(ClipDatabase&& other) noexcept;
    ClipDatabase& operator=(ClipDatabase&& other) noexcept;

    // 数据库操作
    bool isOpen() const;
    std::string getDatabasePath() const;

    // 文本操作
    bool writeText(const std::string& text);
    bool writeTexts(const std::vector<std::string>& texts);

    // 数据读取
    std::vector<std::tuple<int, std::string, int, std::string>> readAllData();

    // 标记操作
    bool setMarkById(int id, int mark);
    bool setMarkByText(const std::string& text, int mark);

    // 删除操作
    bool deleteById(int id);
    bool clearAllData();

    // 更新操作
    bool updateTextById(int id, const std::string& newText);

    // 统计信息
    std::tuple<int, std::string, std::string> getStats();
};
#endif // DATABASE_H
