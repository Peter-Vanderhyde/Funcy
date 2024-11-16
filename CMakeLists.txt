# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(Funcy VERSION 2.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/library.cpp
    src/lexer.cpp
)

# Add source files
add_executable(Funcy ${SOURCES})

# Include directories
target_include_directories(Funcy PRIVATE include)