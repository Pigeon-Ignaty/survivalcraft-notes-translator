cmake_minimum_required(VERSION 3.14)
project(SCNotesTranslator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(Qt5 COMPONENTS Core Gui Widgets WebEngineWidgets LinguistTools REQUIRED)

# Собираем miniz как отдельную библиотеку
add_subdirectory(libs/miniz)
add_subdirectory(src/Help)

set(SOURCES
    src/main.cpp
    src/about.cpp
    src/debug.cpp
    src/mainwindow.cpp
    src/qtexteditstream.cpp
    libs/tiny/tinyxml2.cpp
    version.rc
)

set(HEADERS
    src/about.h
    src/debug.h
    src/mainwindow.h
    src/qtexteditstream.h
    libs/tiny/tinyxml2.h
)

set(FORMS
    ui/debug.ui
)

set(RESOURCES
    resources/files.qrc
)

set(TS_FILES
    resources/translations/SCNotesTranslator_ru.ts
)

qt5_add_translation(QM_FILES ${TS_FILES})

add_custom_target(translations ALL DEPENDS ${QM_FILES})

qt5_wrap_ui(UI_HEADERS ${FORMS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
    ${RESOURCES}
    ${QM_FILES}
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

target_include_directories(SCNotesTranslator PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Help  # чтобы видеть исходники dll
)

#Копируем html справку к exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/guide"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/guide"
)

# После сборки Help.dll — копируем её рядом с SCNotesTranslator.exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Help>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

# Копируем готовые .qm рядом с exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QM_FILES}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
