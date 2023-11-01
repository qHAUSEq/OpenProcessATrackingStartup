cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

set(PROJECT_NAME Monitor)
project(${PROJECT_NAME})

set(SOURCE sources/main.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cfg.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/TXT/)

add_executable(${PROJECT_NAME} ${SOURCE})