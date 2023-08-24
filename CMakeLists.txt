cmake_minimum_required(VERSION 3.20)

project(blackhole_os VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS off)
set(CMAKE_EXE_LINKER_FLAGS "-static")

#file(GLOB example_files "examples/*")

add_executable(assembler "src/assembler.cpp")

add_executable(emulator "src/emulator.cpp")
target_link_libraries(emulator ncurses)
target_compile_options(emulator PUBLIC "-DNCURSES_STATIC")

add_executable(translator "src/translator.cpp")

#file(COPY ${example_files} DESTINATION ${PROJECT_BINARY_DIR})