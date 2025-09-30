# BearLang
cmake_minimum_required(VERSION 3.10)

project(BearLang)
set(EXECUTABLE br)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.c)

set(CMAKE_C_FLAGS_DEBUG   "-Wall -Wextra -Wpedantic -g -gdwarf-4" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)

add_executable(${EXECUTABLE} ${SRC})
target_include_directories(${EXECUTABLE} PRIVATE src)
target_include_directories(${EXECUTABLE} PUBLIC include)
