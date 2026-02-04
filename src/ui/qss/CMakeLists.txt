# 设置部件源文件
set(WIDGETS_SOURCES
    default.h
)

# 创建静态库
add_library(copya_qss STATIC ${WIDGETS_SOURCES})

# 包含目录
target_include_directories(copya_qss PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(copya_qss PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)
