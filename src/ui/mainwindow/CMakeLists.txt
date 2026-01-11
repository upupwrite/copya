# 设置主窗口源文件
set(MAINWINDOW_SOURCES
    copya.cpp
    copya.h
    copya.ui
)

# 创建静态库
add_library(copya_mainwindow STATIC ${MAINWINDOW_SOURCES})

# 包含目录 - 确保包含core目录
target_include_directories(copya_mainwindow PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core  # 指向core目录
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets  # 指向widgets目录
)

# 链接库 - 必须链接到copya_core
target_link_libraries(copya_mainwindow PRIVATE
    copya_core
    Qt${QT_VERSION_MAJOR}::Widgets
)