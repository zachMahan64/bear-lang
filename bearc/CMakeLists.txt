# BearLang
cmake_minimum_required(VERSION 3.20)

project(BearC)
set(EXECUTABLE bearc)
set(BEARC_LIB libbearc)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

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

    src/compiler/ast/printer.c
    src/compiler/ast/ast.c

    src/compiler/diagnostics/error_list.c
    src/compiler/diagnostics/error_codes.c
    src/compiler/diagnostics/src_view.c

    src/compiler/parser/token_eaters.c
    src/compiler/parser/rules.c
    src/compiler/parser/parse_expr.c
    src/compiler/parser/parse_stmt.c
    src/compiler/parser/parse_type.c
    src/compiler/parser/parse_token_slice.c
    src/compiler/parser/parser.c

    src/compiler/hir/span.c
    src/compiler/hir/tables.c

    src/utils/strimap.c
    src/utils/mapu32u32.c
    src/utils/vector.c
    src/utils/arena.c
    src/utils/string.c
    src/utils/file_io.c
    src/utils/spill_arr.c
    src/utils/ansi_codes.c

    src/compiler/debug.c

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
    "-Wall -Wextra -Wpedantic -g -gdwarf-4 -O1" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_BUILD)
endif()

option(BUILD_LIB "Build a static archive: libbearc" OFF)
if(BUILD_LIB)
    message(STATUS "[INFO] libbearc build enabled.")
    add_library(${BEARC_LIB} ${SRC})
    target_include_directories(${BEARC_LIB} PRIVATE src)
    target_include_directories(${BEARC_LIB} PUBLIC include)
    target_include_directories(${BEARC_LIB} PUBLIC include/bearc)
endif()
option(TEST "build tests instead of main" OFF)
if(TEST)
    message(STATUS "[INFO] test build enabled.")
    add_executable(${EXECUTABLE} ${TEST_SRC})
else()
    message(STATUS "[INFO] bearc executable build enabled.")
    add_executable(${EXECUTABLE} ${NO_TEST_SRC})
endif()

target_include_directories(${EXECUTABLE} PRIVATE src)
target_include_directories(${EXECUTABLE} PUBLIC include)
target_include_directories(${EXECUTABLE} PUBLIC include/bearc)


#LLVM

# until LLVM integration is actually needed, toggle this on/off as needed
option(NO_LLVM "Build without LLVM backend" OFF)

if (NOT NO_LLVM)
    # LLVM setup
    set(LLVM_DIR "${CMAKE_SOURCE_DIR}/bearc/llvm/lib/cmake/llvm")

    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "[INFO] found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "[INFO] using LLVMConfig.cmake in: ${LLVM_DIR}")

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
    message(STATUS "[INFO] building WITHOUT LLVM backend (NO_LLVM=ON)")
endif()

