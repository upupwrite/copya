# 设置核心模块源文件
set(CORE_SOURCES
    clipboard.cpp
    clipboard.h
    database.cpp
    database.h
)

# 创建静态库
add_library(copya_core STATIC ${CORE_SOURCES})

# 包含目录
target_include_directories(copya_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(copya_core PRIVATE
    Qt${QT_VERSION_MAJOR}::Network
    sqlite3
)