# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,8388608")

# Project name and version
project(Funcy VERSION 2.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")

# Add source files
add_executable(Funcy ${SOURCES})

# Include directories
target_include_directories(Funcy PRIVATE include)