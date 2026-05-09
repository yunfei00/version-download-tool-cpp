cmake_minimum_required(VERSION 3.21)

project(VersionDownloadTool VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(VersionDownloadTool
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(VersionDownloadTool PRIVATE Qt6::Widgets)

set_target_properties(VersionDownloadTool PROPERTIES
    WIN32_EXECUTABLE TRUE
)
