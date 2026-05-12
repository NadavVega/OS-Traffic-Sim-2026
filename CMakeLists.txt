cmake_minimum_required(VERSION 3.25)
project(OS_Traffic_Simulation C)

set(CMAKE_C_STANDARD 11)

#for macbook users only (you can delete it)
#list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

find_package(raylib REQUIRED)

include_directories(src)

set(SOURCES
        src/main.c
        src/dijkstra.c
        src/dijkstra.h
        src/gui.c
        src/gui.h
)

add_executable(OS-Project ${SOURCES})

target_link_libraries(OS-Project raylib m)