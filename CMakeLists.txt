cmake_minimum_required(VERSION 3.21)

project(QtDesktopApp VERSION 0.1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(QtDesktopApp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(QtDesktopApp PRIVATE Qt6::Widgets)

target_compile_features(QtDesktopApp PRIVATE cxx_std_17)
