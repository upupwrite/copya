# 设置部件源文件
set(WIDGETS_SOURCES
    customtreewidget.cpp
    customtreewidget.h
)

# 创建静态库
add_library(copya_widgets STATIC ${WIDGETS_SOURCES})

# 包含目录
target_include_directories(copya_widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(copya_widgets PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)