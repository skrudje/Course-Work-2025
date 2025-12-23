cmake_minimum_required(VERSION 3.10)

project(NetScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Network)

# 1. Говорим CMake, где искать заголовки (.h)
include_directories(include)

# 2. Указываем файлы исходного кода из папки src/
add_executable(NetScanner
    src/main.cpp
    src/mainwindow.cpp
    include/mainwindow.h
    src/dbmanager.cpp
    include/dbmanager.h
    src/scanner.cpp
    include/scanner.h
    include/graphview.h
    src/stylehelper.cpp
    include/stylehelper.h
    src/topologywidget.cpp
    include/topologywidget.h
)

target_link_libraries(NetScanner PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)

add_custom_command(TARGET NetScanner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/images
    $<TARGET_FILE_DIR:NetScanner>/images)