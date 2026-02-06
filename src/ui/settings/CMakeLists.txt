set(CMAKE_AUTOUIC ON)
# 设置源文件
set(SETTINGS_SOURCES
    setting.cpp
    setting.h
    setting.ui
    general.ui
    general.cpp
    general.h
    setwidget.h

)
# 创建静态库
add_library(copya_settings STATIC ${SETTINGS_SOURCES})

# 包含目录
target_include_directories(copya_settings PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(copya_settings PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)
