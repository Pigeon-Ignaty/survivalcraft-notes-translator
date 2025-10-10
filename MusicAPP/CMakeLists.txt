cmake_minimum_required(VERSION 3.14)
project(MusicAPP VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(Qt5 COMPONENTS Core Gui Widgets WebEngineWidgets REQUIRED)

# Собираем miniz как отдельную библиотеку
add_subdirectory(libs/miniz)

set(SOURCES
    src/main.cpp
    src/about.cpp
    src/debug.cpp
    src/helpdialog.cpp
    src/mainwindow.cpp
    src/qtexteditstream.cpp
    libs/tiny/tinyxml2.cpp
)

set(HEADERS
    src/about.h
    src/debug.h
    src/helpdialog.h
    src/mainwindow.h
    src/qtexteditstream.h
    libs/tiny/tinyxml2.h
)

set(FORMS
    ui/about.ui
    ui/debug.ui
    ui/helpdialog.ui
)

set(RESOURCES
    resources/files.qrc
)

qt5_wrap_ui(UI_HEADERS ${FORMS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    libs/tiny
    libs/miniz
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineWidgets
    miniz
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/guide"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/guide"
)
