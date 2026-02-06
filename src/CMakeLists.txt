# 包含当前目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 注意：必须按依赖顺序添加子目录
add_subdirectory(core)          # 基础库，没有依赖
add_subdirectory(ui/widgets)    # UI部件，没有依赖
add_subdirectory(ui/mainwindow) # 主窗口，依赖core和widgets
add_subdirectory(ui/settings)   # 设置，没有依赖
add_subdirectory(ui/qss)

# 添加资源文件
set(RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/resources/icon.qrc
)

# 根据Qt版本创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(copya
        MANUAL_FINALIZATION
        main.cpp
        ${RESOURCE_FILES}
    )
    
    qt_finalize_executable(copya)
else()
    add_executable(copya
        main.cpp
        ${RESOURCE_FILES}
    )
endif()

# 设置翻译相关变量
set(TS_FILES
    translations/${PROJECT_NAME}_zh_CN.ts
    translations/${PROJECT_NAME}_en_US.ts
    # 添加更多语言文件
)

set(QM_FILES "")
foreach(ts_file ${TS_FILES})
    get_filename_component(qm_file ${ts_file} NAME_WE)
    set(qm_file "${CMAKE_CURRENT_BINARY_DIR}/${qm_file}.qm")
    list(APPEND QM_FILES ${qm_file})

    # 生成翻译文件
    add_custom_command(
        OUTPUT ${qm_file}
        COMMAND Qt6::lrelease
        ARGS ${ts_file} -qm ${qm_file}
        DEPENDS ${ts_file}
        VERBATIM
    )
endforeach()

# 添加自定义目标来生成翻译文件
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# 将生成的.qm文件添加到资源系统
qt_add_resources(${PROJECT_NAME} "translations"
    PREFIX "/translations"
    FILES ${QM_FILES}
)


# 链接库 - 注意顺序也很重要
target_link_libraries(copya PRIVATE
    copya_mainwindow  # 包含了对copya_core的依赖
    copya_settings
    copya_widgets
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    sqlite3
)

# 包含目录
target_include_directories(copya PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/mainwindow
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/settings
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets
)

# 编译选项
target_compile_options(copya PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
