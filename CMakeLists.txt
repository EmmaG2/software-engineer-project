cmake_minimum_required(VERSION 3.21)

project(QtDesktopApp VERSION 0.1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

qt_add_executable(QtDesktopApp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/style/constants.cpp
    src/style/constants.h
    src/resources.qrc
)

target_link_libraries(QtDesktopApp PRIVATE Qt6::Widgets Qt6::Svg)

target_compile_features(QtDesktopApp PRIVATE cxx_std_17)
