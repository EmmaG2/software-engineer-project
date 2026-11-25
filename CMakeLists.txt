cmake_minimum_required(VERSION 3.21)

project(QtDesktopApp VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Sql Gui Core)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Aplicación Principal GUI
qt_add_executable(QtDesktopApp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/style/constants.cpp
    src/style/constants.h
    src/models/models.h
    src/db/databasemanager.h
    src/db/databasemanager.cpp
    src/services/peps_engine.h
    src/services/peps_engine.cpp
    src/services/pdf_generator.h
    src/services/pdf_generator.cpp
    src/ui/logindialog.h
    src/ui/logindialog.cpp
    src/resources.qrc
)

target_link_libraries(QtDesktopApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::Sql
)

target_compile_features(QtDesktopApp PRIVATE cxx_std_20)

# Suite de Pruebas Unitarias (Unidad 3 / RNF-05)
enable_testing()

add_executable(TestPeps
    tests/test_peps.cpp
    src/models/models.h
    src/db/databasemanager.h
    src/db/databasemanager.cpp
    src/services/peps_engine.h
    src/services/peps_engine.cpp
)

target_link_libraries(TestPeps PRIVATE
    Qt6::Core
    Qt6::Sql
)

target_compile_features(TestPeps PRIVATE cxx_std_20)

add_test(NAME TestPeps COMMAND TestPeps)
