cmake_minimum_required(VERSION 3.14)
project(SCNotesTranslator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets WebEngineWidgets LinguistTools Test REQUIRED)

set(XML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/resources/xml)
add_compile_definitions(XML_DIR="${XML_DIR}")

option(BUILD_HELP_DLL "Build Help DLL" OFF)

add_subdirectory(libs/miniz)
if(BUILD_HELP_DLL)
    add_subdirectory(src/Help)
endif()

set(SOURCES
    src/about.cpp
    src/debug.cpp
    src/MusicXmlReader.cpp
    src/mainwindow.cpp
    src/qtexteditstream.cpp
    libs/tiny/tinyxml2.cpp
)

set(HEADERS
    src/about.h
    src/debug.h
    src/MusicXmlReader.h
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

# ---- 1. Библиотека с логикой перевода ----
add_library(SCNotesTranslatorLib STATIC
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
    ${RESOURCES}
    src/TranslateMusic.h src/TranslateMusic.cpp
)

target_include_directories(SCNotesTranslatorLib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/tiny
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/miniz
)

target_link_libraries(SCNotesTranslatorLib
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineWidgets
    miniz
)

# ---- 2. Основное приложение ----
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    version.rc
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SCNotesTranslatorLib
    Qt5::Test
)

# ---- 3. Тест приложения ----
enable_testing()

set(TEST_RESOURCES
    tests/resources/testXml.qrc
    #resources/translations.qrc
)

qt5_add_resources(TEST_RESOURCES_ADDED ${TEST_RESOURCES})

add_executable( SCNotesTranslatorTest
    tests/test_SCNotesTranslator.cpp
    tests/test_SCNotesTranslator.h
    ${TEST_SOURCES}
    ${TEST_RESOURCES_ADDED}
)
target_link_libraries(SCNotesTranslatorTest PRIVATE
    SCNotesTranslatorLib
    Qt5::Test
)

target_include_directories(SCNotesTranslatorTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    libs/tiny
    libs/miniz
)

add_test(NAME IntegrationGuiLogicTest COMMAND SCNotesTranslatorTest)

# ---- Help DLL ----
if(BUILD_HELP_DLL)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Help
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/guide"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/guide"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Help>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
endif()

# ---- Перенос .qm ----
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QM_FILES}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
