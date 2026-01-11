# 包含当前目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 添加UI子目录
add_subdirectory(mainwindow)
add_subdirectory(settings)
add_subdirectory(widgets)

# 创建UI库（可选，用于统一管理UI组件）
add_library(copya_ui INTERFACE)

# 链接所有UI组件
target_link_libraries(copya_ui INTERFACE
    copya_mainwindow
    copya_settings
    copya_widgets
    Qt${QT_VERSION_MAJOR}::Widgets
)