cmake_minimum_required(VERSION 3.14)

project(Help LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets WebEngineWidgets)

set(SOURCES
    Help.cpp
)

set(HEADERS
    Help.h
    help_global.h
)

add_library(Help SHARED
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(Help PRIVATE Qt5::Widgets Qt5::WebEngineWidgets)

target_compile_definitions(Help PRIVATE
    HELP_LIBRARY
    QT_DEPRECATED_WARNINGS
)

install(TARGETS Help
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
#Версия библиотеки
set(HELP_VERSION "1.0.0")
add_compile_definitions(HELP_VERSION="${HELP_VERSION}")

