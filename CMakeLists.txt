# Copyright © 2025 Arsenii Motorin
# Licensed under the Apache License, Version 2.0
# See: http://www.apache.org/licenses/LICENSE-2.0

cmake_minimum_required(VERSION 3.10)
project(dialscript C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Source files
set(SOURCES
        main/main.c
        compiler/compiler.c
        compiler/parser.c
        compiler/verbose.c
        compiler/fixer.c
)

# Headers
set(HEADERS
        main/main.h
        compiler/compiler.h
        compiler/parser.h
        compiler/verbose.h
)

# Executable
add_executable(dialscript ${SOURCES} ${HEADERS})

# Custom target for running
add_custom_target(run
        COMMAND dialscript ${ARGS}
        DEPENDS dialscript
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
