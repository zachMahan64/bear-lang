# BearLang
cmake_minimum_required(VERSION 3.20)

project(BearC)
set(EXECUTABLE bearc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# for globbing:
# file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.c)

set(SRC
    src/compiler/compile.c
    src/compiler/lexer.c
    src/compiler/token.c
    src/compiler/diagnostics/error_list.c
    src/compiler/diagnostics/error_codes.c
    src/compiler/diagnostics/src_view.c
    src/compiler/parser/parser.c

    src/utils/strimap.c
    src/utils/vector.c
    src/utils/arena.c
    src/utils/string.c
    src/utils/file_io.c

    src/cli/bearc.c
    src/cli/args.c
)

set(NO_TEST_SRC
    ${SRC}
    src/main.c
)

set(TEST_SRC
    ${SRC}
    src/tests/test.c
)

set(CMAKE_C_FLAGS_DEBUG
    "-Wall -Wextra -Wpedantic -g -gdwarf-4" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_BUILD)
endif()

option(TEST "Build tests instead of main" OFF)
if(TEST)
    add_executable(${EXECUTABLE} ${TEST_SRC})
else()
    add_executable(${EXECUTABLE} ${NO_TEST_SRC})
endif()

target_include_directories(${EXECUTABLE} PRIVATE src)
target_include_directories(${EXECUTABLE} PUBLIC include)


#LLVM

# until LLVM integration is actually needed, toggle this on/off as needed
option(NO_LLVM "Build without LLVM backend" OFF)

if (NOT NO_LLVM)
    # LLVM setup
    set(LLVM_DIR "${CMAKE_SOURCE_DIR}/bearc/llvm/lib/cmake/llvm")

    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    target_include_directories(${EXECUTABLE} PRIVATE ${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    llvm_map_components_to_libnames(LLVM_LIBS
        Core
        Support
        Option
        IRReader
    )

    target_link_libraries(${EXECUTABLE} PRIVATE ${LLVM_LIBS})
else()
    message(STATUS "Building WITHOUT LLVM backend (NO_LLVM=ON)")
endif()

