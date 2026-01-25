#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

class ClipDatabase
{
private:
    sqlite3 *db;
    std::string dbPath;

    // 私有静态方法
    static bool createDirectoryIfNotExists(const std::string &path);
    static std::string expandHomePath(const std::string &path);

    // 私有成员方法
    void logError(const std::string &operation,
                  const std::string &errorMsg) const;
    void logSuccess(const std::string &operation) const;
    std::string escapeLikePattern(const std::string &pattern) const;

public:
    // 构造函数和析构函数
    explicit ClipDatabase(const std::string &databaseName = "~/.copya/user.db");
    ~ClipDatabase();

    // 禁用拷贝构造和赋值
    ClipDatabase(const ClipDatabase &) = delete;
    ClipDatabase &operator=(const ClipDatabase &) = delete;

    // 移动语义
    ClipDatabase(ClipDatabase &&other) noexcept;
    ClipDatabase &operator=(ClipDatabase &&other) noexcept;

    // 数据库操作
    bool isOpen() const;
    std::string getDatabasePath() const;

    // 文本操作
    bool writeText(const std::string &text);
    bool writeTexts(const std::vector<std::string> &texts);

    // 数据读取
    std::vector<std::tuple<int, std::string, int, std::string>> readAllData();

    // 标记操作
    bool setMarkById(int id, int mark);
    bool setMarkByText(const std::string &text, int mark);

    // 删除操作
    bool deleteById(int id);
    bool clearAllData();

    // 更新操作
    bool updateTextById(int id, const std::string &newText);

    // 统计信息
    std::tuple<int, std::string, std::string> getStats();

    std::vector<std::tuple<int, std::string, int, std::string>> searchText(
        const std::string &searchTerm, bool caseSensitive = false,
        bool exactMatch = false);

    // 设置标签文本，自动分配mark并返回
    int setLabel(const std::string &label);

    // 设置项目的mark
    bool setMark(int id, int mark);

    // 载入最新的N条项目
    std::vector<std::tuple<int, std::string, int, std::string>> readRecentData(
        int limit = 150);

    // 获取mark对应的所有项目
    std::vector<std::tuple<int, std::string, int, std::string>> getDataByMark(
        int mark);

    // 读取所有mark和对应的标签文本
    std::vector<std::pair<int, std::string>> getAllMarksAndLabels();

    // 更新标签文本
    bool updateLabel(int mark, const std::string &newLabel);

    // 更新id的时间戳（修复现有问题）
    bool updateTimestampById(int id);

    // 删除标签
    bool deleteMark(int mark);

    // 获取最新的文本
    std::string readLatestText();

    // 根据ID获取文本内容
    std::string getTextById(int id);

    // 根据ID获取完整记录（文本、mark、时间）
    std::tuple<bool, std::string, int, std::string> getFullRecordById(int id);
};
#endif  // DATABASE_H
