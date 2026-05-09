cmake_minimum_required(VERSION 3.21)

project(VersionDownloadTool VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(VersionDownloadTool
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/core/RemoteFileItem.h
    src/core/RemoteScanner.h
    src/core/RemoteScanner.cpp
    src/core/DownloadTask.h
    src/core/DownloadTask.cpp
    src/core/DownloadManager.h
    src/core/DownloadManager.cpp
    src/ui/DownloadTableModel.h
    src/ui/DownloadTableModel.cpp
)

target_link_libraries(VersionDownloadTool PRIVATE Qt6::Widgets Qt6::Network)

set_target_properties(VersionDownloadTool PROPERTIES
    WIN32_EXECUTABLE TRUE
)
