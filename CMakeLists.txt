cmake_minimum_required(VERSION 3.16)

project(copya VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt相关设置
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network LinguistTools)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找lrelease工具
if(QT_VERSION_MAJOR EQUAL 6)
    get_target_property(QT_LRELEASE_EXECUTABLE Qt6::lrelease LOCATION)
elseif(QT_VERSION_MAJOR EQUAL 5)
    get_target_property(QT_LRELEASE_EXECUTABLE Qt5::lrelease LOCATION)
endif()

# 创建翻译文件目录
set(TS_FILES_DIR "${CMAKE_SOURCE_DIR}/translations")
set(QM_FILES_DIR "${CMAKE_BINARY_DIR}/translations")

# 确保输出目录存在
file(MAKE_DIRECTORY ${QM_FILES_DIR})

# 查找所有.ts文件
file(GLOB TS_FILES "${TS_FILES_DIR}/*.ts")

# 为每个.ts文件创建自定义命令来生成.qm文件
foreach(TS_FILE ${TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    set(QM_FILE "${QM_FILES_DIR}/${TS_NAME}.qm")

    # 添加自定义命令来运行lrelease
    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND ${QT_LRELEASE_EXECUTABLE}
        ARGS ${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${TS_FILE}
        VERBATIM
    )

    # 将生成的.qm文件添加到列表
    list(APPEND QM_FILES ${QM_FILE})
endforeach()

# 添加自定义目标，以便可以手动重新编译翻译文件
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# 添加子目录
add_subdirectory(src)

# 安装翻译文件（可选）
install(DIRECTORY ${QM_FILES_DIR}/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/translations
    FILES_MATCHING PATTERN "*.qm"
)
