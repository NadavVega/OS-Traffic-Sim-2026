cmake_minimum_required(VERSION 3.25)
project(OS_Traffic_Simulation C)

set(CMAKE_C_STANDARD 11)

#for macbook users only (you can delete it)
#list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

find_package(raylib REQUIRED)

include_directories(include)

set(SOURCES
        src/main.c
        src/gui.c
        include/gui.h

)

add_executable(OS_Project ${SOURCES})

target_link_libraries(OS_Project raylib m)