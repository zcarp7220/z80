cmake_minimum_required(VERSION 3.10)
project(pacman LANGUAGES C)
add_executable(pacman 
src/common.h
src/main.c
src/z80.c
src/z80.h
)
target_compile_options(pacman PRIVATE -Wall -Wextra -Wpedantic -lSDL3)
