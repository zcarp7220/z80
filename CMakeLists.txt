cmake_minimum_required(VERSION 3.10)
project(pacman LANGUAGES C)
option(PACMAN_VENDORED "Use vendored libraries" OFF)
if(MYGAME_VENDORED)
    # This assumes you have added SDL as a submodule in vendored/SDL
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL3 package,
    # 2. look for the SDL3-shared component, and
    # 3. fail if the shared component cannot be found.
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()
add_executable(pacman 
src/common.h
src/main.c
src/z80.c
src/z80.h
)
target_compile_options(pacman PRIVATE -Wall -Wextra -Wpedantic -lSDL3)
target_link_libraries(pacman PRIVATE SDL3::SDL3)
