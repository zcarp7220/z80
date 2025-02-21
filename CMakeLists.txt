cmake_minimum_required(VERSION 3.10)
project(z80 LANGUAGES C)

add_executable(z80 
src/z80.c
src/z80.h
src/common.h
src/main.c
src/opcodes.c
)
find_package(json-c CONFIG)
target_link_libraries(${PROJECT_NAME} PRIVATE json-c::json-c)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
