#include "database.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

// 静态私有方法实现
bool ClipDatabase::createDirectoryIfNotExists(const std::string &path)
{
    try
    {
        fs::create_directories(fs::path(path));
        return true;
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "创建目录失败: " << e.what() << std::endl;
        return false;
    }
}

std::string ClipDatabase::expandHomePath(const std::string &path)
{
    if (path.empty())
        return path;

    // 处理 ~ 开头的路径
    if (path[0] == '~')
    {
        const char *home = std::getenv("HOME");
        if (home)
        {
            std::string homeStr(home);
            // 确保路径分隔符正确
            if (path.size() > 1 && path[1] == '/')
            {
                return homeStr + path.substr(1);
            }
            else
            {
                return homeStr + path.substr(1);
            }
        }
    }

    return path;
}

// 日志记录方法
void ClipDatabase::logError(const std::string &operation,
                            const std::string &errorMsg) const
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    std::cerr << "[" << oss.str() << "] "
              << "操作: " << operation << ", 错误: " << errorMsg
              << " (数据库: " << dbPath << ")" << std::endl;
}

void ClipDatabase::logSuccess(const std::string &operation) const
{
    std::cout << "操作成功: " << operation << " (数据库: " << dbPath << ")"
              << std::endl;
}

// 构造函数
ClipDatabase::ClipDatabase(const std::string &databaseName) : db(nullptr)
{
    // 扩展路径
    dbPath = expandHomePath(databaseName);

    // 确保目录存在
    fs::path fullPath(dbPath);
    if (fullPath.has_parent_path())
    {
        std::string parentDir = fullPath.parent_path().string();
        if (!createDirectoryIfNotExists(parentDir))
        {
            std::cerr << "无法创建数据库目录: " << parentDir << std::endl;
            return;
        }
    }

    // 打开数据库
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK)
    {
        std::string errorMsg = sqlite3_errmsg(db);
        logError("打开数据库", errorMsg);

        // 关闭数据库连接（如果有部分打开）
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
        return;
    }

    // 启用外键约束
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr,
                      nullptr);
    if (rc != SQLITE_OK)
    {
        logError("启用外键约束", sqlite3_errmsg(db));
    }

    // 在构造函数中修改建表语句
    const char *sql =
        "CREATE TABLE IF NOT EXISTS texts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "text_content TEXT NOT NULL, "
        "mark INTEGER DEFAULT 0, "
        "created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "updated_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"

        "CREATE TABLE IF NOT EXISTS marks ("
        "mark INTEGER PRIMARY KEY, "
        "label TEXT NOT NULL, "
        "created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"

        "CREATE INDEX IF NOT EXISTS idx_texts_created_time "
        "ON texts(created_time);"

        "CREATE INDEX IF NOT EXISTS idx_texts_mark "
        "ON texts(mark);"

        "CREATE INDEX IF NOT EXISTS idx_marks_label "
        "ON marks(label);";

    char *errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        logError("创建表", errMsg ? errMsg : "未知错误");
        if (errMsg)
            sqlite3_free(errMsg);

        sqlite3_close(db);
        db = nullptr;
        return;
    }

    logSuccess("数据库初始化");
}

// 析构函数
ClipDatabase::~ClipDatabase()
{
    if (db)
    {
        // 执行清理操作
        sqlite3_exec(db, "PRAGMA optimize;", nullptr, nullptr, nullptr);

        // 关闭数据库
        int rc = sqlite3_close(db);
        if (rc != SQLITE_OK)
        {
            logError("关闭数据库", sqlite3_errmsg(db));
        }
        else
        {
            std::cout << "数据库连接已关闭: " << dbPath << std::endl;
        }
    }
}

// 移动构造函数
ClipDatabase::ClipDatabase(ClipDatabase &&other) noexcept
    : db(other.db), dbPath(std::move(other.dbPath))
{
    other.db = nullptr;
}

// 移动赋值运算符
ClipDatabase &ClipDatabase::operator=(ClipDatabase &&other) noexcept
{
    if (this != &other)
    {
        if (db)
        {
            sqlite3_close(db);
        }
        db = other.db;
        dbPath = std::move(other.dbPath);
        other.db = nullptr;
    }
    return *this;
}

// 检查数据库是否打开
bool ClipDatabase::isOpen() const { return db != nullptr; }

// 获取数据库路径
std::string ClipDatabase::getDatabasePath() const { return dbPath; }

// 写入文本到数据库
bool ClipDatabase::writeText(const std::string &text)
{
    if (!db)
    {
        logError("写入文本", "数据库未打开");
        return false;
    }

    if (text.empty())
    {
        logError("写入文本", "文本内容为空");
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    std::string sql = "INSERT INTO texts (text_content) VALUES (?);";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备写入语句", sqlite3_errmsg(db));
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, text.c_str(), -1, SQLITE_TRANSIENT);

    // 执行
    rc = sqlite3_step(stmt);

    // 清理
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行写入", sqlite3_errmsg(db));
        return false;
    }

    logSuccess("写入文本");
    return true;
}

// 批量写入文本
bool ClipDatabase::writeTexts(const std::vector<std::string> &texts)
{
    if (!db)
    {
        logError("批量写入文本", "数据库未打开");
        return false;
    }

    if (texts.empty())
    {
        logError("批量写入文本", "文本列表为空");
        return false;
    }

    // 开始事务
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        logError("开始事务", errMsg ? errMsg : "未知错误");
        if (errMsg)
            sqlite3_free(errMsg);
        return false;
    }

    bool success = true;
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "INSERT INTO texts (text_content) VALUES (?);";

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备批量写入语句", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    for (const auto &text : texts)
    {
        if (text.empty())
            continue;

        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, text.c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            logError("批量写入单条文本", sqlite3_errmsg(db));
            success = false;
            break;
        }
    }

    sqlite3_finalize(stmt);

    if (success)
    {
        rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            logError("提交事务", errMsg ? errMsg : "未知错误");
            if (errMsg)
                sqlite3_free(errMsg);
            success = false;
        }
        else
        {
            logSuccess("批量写入文本 (" + std::to_string(texts.size()) + "条)");
        }
    }
    else
    {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }

    return success;
}

// 读取所有数据
std::vector<std::tuple<int, std::string, int, std::string>>
ClipDatabase::readAllData()
{
    std::vector<std::tuple<int, std::string, int, std::string>> results;

    if (!db)
    {
        logError("读取所有数据", "数据库未打开");
        return results;
    }

    const char *sql =
        "SELECT id, text_content, mark, created_time "
        "FROM texts ORDER BY created_time ASC;";
    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备读取语句", sqlite3_errmsg(db));
        return results;
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);

        const unsigned char *text = sqlite3_column_text(stmt, 1);
        std::string textStr = text ? reinterpret_cast<const char *>(text) : "";

        int mark = sqlite3_column_int(stmt, 2);

        const unsigned char *time = sqlite3_column_text(stmt, 3);
        std::string timeStr = time ? reinterpret_cast<const char *>(time) : "";

        results.emplace_back(id, textStr, mark, timeStr);
        count++;
    }

    sqlite3_finalize(stmt);

    if (count > 0)
    {
        logSuccess("读取数据 (" + std::to_string(count) + "条)");
    }

    return results;
}

// 根据ID设置标记（调用新的setMark函数）
bool ClipDatabase::setMarkById(int id, int mark) { return setMark(id, mark); }
// 根据文本内容设置标记
bool ClipDatabase::setMarkByText(const std::string &text, int mark)
{
    if (!db)
    {
        logError("设置标记(文本)", "数据库未打开");
        return false;
    }

    if (text.empty())
    {
        logError("设置标记(文本)", "文本内容为空");
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    std::string sql =
        "UPDATE texts SET mark = ?, updated_time = CURRENT_TIMESTAMP "
        "WHERE text_content = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备更新标记语句(文本)", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, mark);
    sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行更新标记(文本)", sqlite3_errmsg(db));
        return false;
    }

    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        logError("设置标记(文本)", "未找到文本内容为 \"" + text + "\" 的记录");
        return false;
    }

    logSuccess("设置标记(文本: \"" + text +
               "\", Mark: " + std::to_string(mark) + ")");
    return true;
}

// 根据ID删除记录
bool ClipDatabase::deleteById(int id)
{
    if (!db)
    {
        logError("删除记录", "数据库未打开");
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    std::string sql = "DELETE FROM texts WHERE id = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备删除语句", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行删除", sqlite3_errmsg(db));
        return false;
    }

    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        logError("删除记录", "未找到ID为 " + std::to_string(id) + " 的记录");
        return false;
    }

    logSuccess("删除记录(ID: " + std::to_string(id) + ")");
    return true;
}

// 清空所有数据
bool ClipDatabase::clearAllData()
{
    if (!db)
    {
        logError("清空数据", "数据库未打开");
        return false;
    }

    char *errMsg = nullptr;
    std::string sql = "DELETE FROM texts;";

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        logError("清空数据", errMsg ? errMsg : "未知错误");
        if (errMsg)
            sqlite3_free(errMsg);
        return false;
    }

    // 重置自增ID
    sql = "DELETE FROM sqlite_sequence WHERE name='texts';";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    logSuccess("清空所有数据");
    return true;
}

// 获取数据库统计信息
std::tuple<int, std::string, std::string> ClipDatabase::getStats()
{
    int totalRecords = 0;
    std::string oldestTime = "N/A";
    std::string newestTime = "N/A";

    if (!db)
    {
        return std::make_tuple(totalRecords, oldestTime, newestTime);
    }

    // 获取总记录数
    const char *countSql = "SELECT COUNT(*) FROM texts;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, countSql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            totalRecords = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // 获取最早和最晚时间
    const char *timeSql =
        "SELECT MIN(created_time), MAX(created_time) FROM texts;";
    if (sqlite3_prepare_v2(db, timeSql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char *minTime = sqlite3_column_text(stmt, 0);
            const unsigned char *maxTime = sqlite3_column_text(stmt, 1);

            if (minTime)
                oldestTime = reinterpret_cast<const char *>(minTime);
            if (maxTime)
                newestTime = reinterpret_cast<const char *>(maxTime);
        }
        sqlite3_finalize(stmt);
    }

    return std::make_tuple(totalRecords, oldestTime, newestTime);
}

// 根据ID更新文本内容
bool ClipDatabase::updateTextById(int id, const std::string &newText)
{
    if (!db)
    {
        logError("更新文本", "数据库未打开");
        return false;
    }

    if (newText.empty())
    {
        logError("更新文本", "新文本内容为空");
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    std::string sql =
        "UPDATE texts SET text_content = ?, updated_time = CURRENT_TIMESTAMP "
        "WHERE id = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备更新文本语句", sqlite3_errmsg(db));
        return false;
    }

    // 绑定参数：第一个参数是新文本，第二个参数是ID
    sqlite3_bind_text(stmt, 1, newText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行更新文本", sqlite3_errmsg(db));
        return false;
    }

    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        logError("更新文本", "未找到ID为 " + std::to_string(id) + " 的记录");
        return false;
    }

    logSuccess("更新文本(ID: " + std::to_string(id) + ")");
    return true;
}

std::vector<std::tuple<int, std::string, int, std::string>>
ClipDatabase::searchText(const std::string &searchTerm, bool caseSensitive,
                         bool exactMatch)
{
    std::vector<std::tuple<int, std::string, int, std::string>> results;

    if (!db)
    {
        logError("搜索文本", "数据库未打开");
        return results;
    }

    if (searchTerm.empty())
    {
        logError("搜索文本", "搜索词为空");
        return results;
    }

    std::string sql;
    sqlite3_stmt *stmt = nullptr;

    if (exactMatch)
    {
        // 精确匹配
        if (caseSensitive)
        {
            sql =
                "SELECT id, text_content, mark, created_time "
                "FROM texts WHERE text_content = ? "
                "ORDER BY created_time DESC;";
        }
        else
        {
            sql =
                "SELECT id, text_content, mark, created_time "
                "FROM texts WHERE LOWER(text_content) = LOWER(?) "
                "ORDER BY created_time DESC;";
        }
    }
    else
    {
        // 模糊匹配 - 搜索内容中的文本
        if (caseSensitive)
        {
            // 区分大小写
            sql =
                "SELECT id, text_content, mark, created_time "
                "FROM texts WHERE text_content LIKE ? ESCAPE '\\' "
                "ORDER BY created_time DESC;";
        }
        else
        {
            // 不区分大小写
            sql =
                "SELECT id, text_content, mark, created_time "
                "FROM texts WHERE LOWER(text_content) LIKE LOWER(?) ESCAPE "
                "'\\' "
                "ORDER BY created_time DESC;";
        }
    }

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备搜索语句", sqlite3_errmsg(db));
        return results;
    }

    if (exactMatch)
    {
        // 精确匹配只需绑定搜索词本身
        sqlite3_bind_text(stmt, 1, searchTerm.c_str(), -1, SQLITE_TRANSIENT);
    }
    else
    {
        // 模糊匹配：构建 LIKE 模式
        std::string searchPattern = "%" + escapeLikePattern(searchTerm) + "%";
        sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_TRANSIENT);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);

        const unsigned char *text = sqlite3_column_text(stmt, 1);
        std::string textStr = text ? reinterpret_cast<const char *>(text) : "";

        int mark = sqlite3_column_int(stmt, 2);

        const unsigned char *time = sqlite3_column_text(stmt, 3);
        std::string timeStr = time ? reinterpret_cast<const char *>(time) : "";

        results.emplace_back(id, textStr, mark, timeStr);
        count++;
    }

    sqlite3_finalize(stmt);

    if (count > 0)
    {
        logSuccess("搜索文本 (" + std::to_string(count) + "条结果, 搜索词: \"" +
                   searchTerm + "\")");
    }
    else
    {
        std::cout << "搜索未找到结果: \"" << searchTerm << "\"" << std::endl;
    }

    return results;
}

// 辅助函数：转义LIKE模式中的特殊字符
std::string ClipDatabase::escapeLikePattern(const std::string &pattern) const
{
    std::string escaped;
    escaped.reserve(pattern.length() * 2);

    for (char c : pattern)
    {
        if (c == '%' || c == '_' || c == '\\')
        {
            escaped.push_back('\\');
        }
        escaped.push_back(c);
    }

    return escaped;
}

// 设置标签文本，自动分配mark并返回
int ClipDatabase::setLabel(const std::string &label)
{
    if (!db)
    {
        logError("设置标签", "数据库未打开");
        return -1;
    }

    if (label.empty())
    {
        logError("设置标签", "标签文本为空");
        return -1;
    }

    // 首先检查标签是否已存在
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT mark FROM marks WHERE label = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备查询标签语句", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, label.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 标签已存在，返回现有的mark
        int existingMark = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        logSuccess("标签已存在 (Mark: " + std::to_string(existingMark) +
                   ", Label: \"" + label + "\")");
        return existingMark;
    }

    sqlite3_finalize(stmt);

    // 标签不存在，自动分配新的mark
    // 查找当前最大的mark
    sql = "SELECT COALESCE(MAX(mark), 0) FROM marks;";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备查询最大mark语句", sqlite3_errmsg(db));
        return -1;
    }

    int newMark = 2;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        newMark = sqlite3_column_int(stmt, 0) + 1;
    }
    sqlite3_finalize(stmt);

    // 插入新的标签
    sql = "INSERT INTO marks (mark, label) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备插入标签语句", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, newMark);
    sqlite3_bind_text(stmt, 2, label.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行插入标签", sqlite3_errmsg(db));
        return -1;
    }

    logSuccess("设置标签 (Mark: " + std::to_string(newMark) + ", Label: \"" +
               label + "\")");
    return newMark;
}

// 设置项目的mark（修复现有问题）
bool ClipDatabase::setMark(int id, int mark)
{
    if (!db)
    {
        logError("设置项目mark", "数据库未打开");
        return false;
    }

    // 检查mark是否存在于marks表中
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT 1 FROM marks WHERE mark = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备检查mark语句", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, mark);

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        logError("设置项目mark",
                 "mark " + std::to_string(mark) + " 不存在于marks表中");
        return false;
    }

    sqlite3_finalize(stmt);

    // 更新texts表中的mark
    sql =
        "UPDATE texts SET mark = ?, updated_time = CURRENT_TIMESTAMP WHERE id "
        "= ?;";

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备更新mark语句", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, mark);
    sqlite3_bind_int(stmt, 2, id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行更新mark", sqlite3_errmsg(db));
        return false;
    }

    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        logError("设置项目mark",
                 "未找到ID为 " + std::to_string(id) + " 的记录");
        return false;
    }

    logSuccess("设置项目mark (ID: " + std::to_string(id) +
               ", Mark: " + std::to_string(mark) + ")");
    return true;
}

// 载入最新的N条项目
std::vector<std::tuple<int, std::string, int, std::string>>
ClipDatabase::readRecentData(int limit)
{
    std::vector<std::tuple<int, std::string, int, std::string>> results;

    if (!db)
    {
        logError("读取最近数据", "数据库未打开");
        return results;
    }

    std::string sql =
        "SELECT id, text_content, mark, created_time "
        "FROM texts ORDER BY created_time DESC LIMIT ?;";

    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备读取最近数据语句", sqlite3_errmsg(db));
        return results;
    }

    sqlite3_bind_int(stmt, 1, limit);

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *text = sqlite3_column_text(stmt, 1);
        std::string textStr = text ? reinterpret_cast<const char *>(text) : "";
        int mark = sqlite3_column_int(stmt, 2);
        const unsigned char *time = sqlite3_column_text(stmt, 3);
        std::string timeStr = time ? reinterpret_cast<const char *>(time) : "";

        results.emplace_back(id, textStr, mark, timeStr);
        count++;
    }

    sqlite3_finalize(stmt);

    if (count > 0)
    {
        logSuccess("读取最近数据 (" + std::to_string(count) + "条)");
    }

    return results;
}

// 获取mark对应的所有项目
std::vector<std::tuple<int, std::string, int, std::string>>
ClipDatabase::getDataByMark(int mark)
{
    std::vector<std::tuple<int, std::string, int, std::string>> results;

    if (!db)
    {
        logError("获取mark数据", "数据库未打开");
        return results;
    }

    std::string sql =
        "SELECT id, text_content, mark, created_time "
        "FROM texts WHERE mark = ? ORDER BY created_time DESC;";

    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备获取mark数据语句", sqlite3_errmsg(db));
        return results;
    }

    sqlite3_bind_int(stmt, 1, mark);

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *text = sqlite3_column_text(stmt, 1);
        std::string textStr = text ? reinterpret_cast<const char *>(text) : "";
        int currentMark = sqlite3_column_int(stmt, 2);
        const unsigned char *time = sqlite3_column_text(stmt, 3);
        std::string timeStr = time ? reinterpret_cast<const char *>(time) : "";

        results.emplace_back(id, textStr, currentMark, timeStr);
        count++;
    }

    sqlite3_finalize(stmt);

    if (count > 0)
    {
        logSuccess("获取mark数据 (Mark: " + std::to_string(mark) + ", " +
                   std::to_string(count) + "条)");
    }

    return results;
}

// 读取所有mark和对应的标签文本
std::vector<std::pair<int, std::string>> ClipDatabase::getAllMarksAndLabels()
{
    std::vector<std::pair<int, std::string>> results;

    if (!db)
    {
        logError("读取所有marks", "数据库未打开");
        return results;
    }

    std::string sql = "SELECT mark, label FROM marks ORDER BY mark ASC;";

    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备读取marks语句", sqlite3_errmsg(db));
        return results;
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int mark = sqlite3_column_int(stmt, 0);
        const unsigned char *label = sqlite3_column_text(stmt, 1);
        std::string labelStr =
            label ? reinterpret_cast<const char *>(label) : "";

        results.emplace_back(mark, labelStr);
        count++;
    }

    sqlite3_finalize(stmt);

    if (count > 0)
    {
        logSuccess("读取所有marks (" + std::to_string(count) + "条)");
    }

    return results;
}

// 更新标签文本
bool ClipDatabase::updateLabel(int mark, const std::string &newLabel)
{
    if (!db)
    {
        logError("更新标签", "数据库未打开");
        return false;
    }

    if (newLabel.empty())
    {
        logError("更新标签", "新标签文本为空");
        return false;
    }

    // 检查mark是否存在
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT 1 FROM marks WHERE mark = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备检查mark语句", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, mark);

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        logError("更新标签", "mark " + std::to_string(mark) + " 不存在");
        return false;
    }

    sqlite3_finalize(stmt);

    // 更新标签文本
    sql = "UPDATE marks SET label = ? WHERE mark = ?;";

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备更新标签语句", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, newLabel.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, mark);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行更新标签", sqlite3_errmsg(db));
        return false;
    }

    logSuccess("更新标签 (Mark: " + std::to_string(mark) + ", New Label: \"" +
               newLabel + "\")");
    return true;
}

// 修复更新id的时间戳函数
bool ClipDatabase::updateTimestampById(int id)
{
    if (!db)
    {
        logError("更新时间戳", "数据库未打开");
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    std::string sql =
        "UPDATE texts SET updated_time = CURRENT_TIMESTAMP WHERE id = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备更新时间戳语句", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行更新时间戳", sqlite3_errmsg(db));
        return false;
    }

    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        logError("更新时间戳", "未找到ID为 " + std::to_string(id) + " 的记录");
        return false;
    }

    logSuccess("更新时间戳 (ID: " + std::to_string(id) + ")");
    return true;
}

bool ClipDatabase::deleteMark(int mark)
{
    if (!db)
    {
        logError("删除mark", "数据库未打开");
        return false;
    }

    // 开始事务
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        logError("开始事务", errMsg ? errMsg : "未知错误");
        if (errMsg)
            sqlite3_free(errMsg);
        return false;
    }

    bool success = true;

    // 1. 将texts表中所有该mark设置为0
    std::string sql = "UPDATE texts SET mark = 0 WHERE mark = ?;";
    sqlite3_stmt *stmt = nullptr;

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        logError("准备更新语句", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    sqlite3_bind_int(stmt, 1, mark);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        logError("执行更新", sqlite3_errmsg(db));
        success = false;
    }

    // 2. 删除marks表中的记录
    if (success)
    {
        sql = "DELETE FROM marks WHERE mark = ?;";
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            logError("准备删除语句", sqlite3_errmsg(db));
            success = false;
        }
        else
        {
            sqlite3_bind_int(stmt, 1, mark);
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);

            if (rc != SQLITE_DONE)
            {
                logError("执行删除", sqlite3_errmsg(db));
                success = false;
            }
        }
    }

    // 提交或回滚事务
    if (success)
    {
        rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            logError("提交事务", errMsg ? errMsg : "未知错误");
            if (errMsg)
                sqlite3_free(errMsg);
            success = false;
        }
        else
        {
            logSuccess("删除mark (" + std::to_string(mark) + ")");
        }
    }
    else
    {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }

    return success;
}
