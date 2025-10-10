#Именно так я подружил эту библиотеку с главным проектом
cmake_minimum_required(VERSION 3.14)
project(miniz C)

set(CMAKE_C_STANDARD 99)

add_library(miniz STATIC miniz.c)

target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(miniz PUBLIC _CRT_SECURE_NO_WARNINGS)

if(MSVC)
    # Линкуем CRT библиотеки, которые содержат localtime64_s и другие функции
    target_link_libraries(miniz PRIVATE legacy_stdio_definitions)
    target_link_libraries(miniz PRIVATE oldnames.lib)
endif()
