cmake_minimum_required(VERSION 4.2)
project(dialscript C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(dialscript main.c)
