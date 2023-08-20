cmake_minimum_required(VERSION 3.20)

project(blackhole_os VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

file(GLOB example_files "examples/*")

add_executable(assembler "src/assembler.cpp")
add_executable(emulator "src/emulator.cpp")
add_executable(translator "src/translator.cpp")
file(COPY ${example_files} DESTINATION ${PROJECT_BINARY_DIR})