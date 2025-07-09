cmake_minimum_required(VERSION 3.10)
project(z80 LANGUAGES C)
add_executable(z80 src/jsonTests.c src/common.h src/main.c src/zex.c z80.c
                   z80.h)

# target_link_options(${PROJECT_NAME} PRIVATE -pg)
target_compile_options(
  ${PROJECT_NAME}
  PRIVATE -Wall
          -Wextra
          -Wpedantic
          -O2
          -lto=auto
          -ldl
          -ggdb)
