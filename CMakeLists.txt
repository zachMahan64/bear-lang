# BearLang
cmake_minimum_required(VERSION 3.10)

project(BearLang)
set(EXECUTABLE bearc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# for globbing:
# file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.c)

set(SRC
    src/compiler/ast/node.c
    src/compiler/ast/node_arena.c
    src/compiler/ast/parser.c
    src/compiler/compile.c
    src/compiler/lexer.c
    src/compiler/token.c
    src/compiler/diagnostics/error_list.c
    src/compiler/diagnostics/error_codes.c
    src/compiler/diagnostics/src_view.c

    src/containers/strimap.c
    src/containers/vector.c
    src/containers/arena.c
    src/containers/string.c

    src/tests/test.c

    src/bearlang.c
    src/cli_args.c
    src/file_io.c
    src/main.c
)

set(CMAKE_C_FLAGS_DEBUG   "-Wall -Wextra -Wpedantic -g -gdwarf-4" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)

add_executable(${EXECUTABLE} ${SRC})
target_include_directories(${EXECUTABLE} PRIVATE src)
target_include_directories(${EXECUTABLE} PUBLIC include)
