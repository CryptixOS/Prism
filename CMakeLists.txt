#*
#* Created by v1tr10l7 on 04.11.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/
#
# C++ modules build of Prism (`import Prism;`).
#
# Meson can't scan C++ module dependencies for GCC, so the module build uses
# CMake (>= 3.28, with the Ninja generator). The header-based build, the
# kernel cross builds and CI keep using meson.build. Both build the same
# sources: the module interface unit (Source/Prism/Prism.cppm) includes the
# headers inside the module purview, and PRISM_EXPORT marks what's exported.
#
#   cmake -S . -B build-modules -G Ninja
#   cmake --build build-modules
#   ctest --test-dir build-modules
#
cmake_minimum_required(VERSION 3.28)
project(Prism VERSION 0.0.1 LANGUAGES C CXX)

if (NOT CMAKE_GENERATOR MATCHES "Ninja")
    message(FATAL_ERROR "C++ modules need the Ninja generator (-G Ninja)")
endif()

option(PRISM_BUILD_TESTS "Build the unit tests against the Prism module" ON)
option(PRISM_WERROR "Treat warnings as errors" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# ------------------------------------------------------------- dependencies
# Reuse the copies meson downloaded into subprojects/ when they exist (the
# fmt one is also patched there), otherwise fetch the same versions.
include(FetchContent)
set(PRISM_SUBPROJECTS ${CMAKE_CURRENT_SOURCE_DIR}/subprojects)

function(prism_dependency name subdir)
    cmake_parse_arguments(ARG "" "URL;URL_HASH;GIT_REPOSITORY;GIT_TAG" "" ${ARGN})
    if (EXISTS ${PRISM_SUBPROJECTS}/${subdir})
        set(${name}_SOURCE_DIR ${PRISM_SUBPROJECTS}/${subdir} PARENT_SCOPE)
        return()
    endif()
    if (ARG_URL)
        FetchContent_Declare(${name} URL ${ARG_URL} URL_HASH ${ARG_URL_HASH})
    else()
        FetchContent_Declare(${name}
            GIT_REPOSITORY ${ARG_GIT_REPOSITORY} GIT_TAG ${ARG_GIT_TAG}
            GIT_SHALLOW TRUE)
    endif()
    # Headers/sources only: don't run their own CMake projects.
    FetchContent_GetProperties(${name})
    if (NOT ${name}_POPULATED)
        FetchContent_Populate(${name})
    endif()
    set(${name}_SOURCE_DIR ${${name}_SOURCE_DIR} PARENT_SCOPE)
endfunction()

prism_dependency(fmt fmt-10.2.0
    URL https://github.com/fmtlib/fmt/archive/10.2.0.tar.gz
    URL_HASH SHA256=3ca91733a7313a8ad41c0885929415f8ec0a2a31d4dc7e27e9331412f4ca26ac)
prism_dependency(magic_enum magic_enum
    URL https://github.com/Neargye/magic_enum/releases/download/v0.9.5/magic_enum-v0.9.5.tar.gz
    URL_HASH SHA256=0694e1ef160cbb6b8be2f43c7df349295911cb7daebdd4e557adaac2c18420f9)
prism_dependency(demangler demangler
    GIT_REPOSITORY https://github.com/ilobilo/demangler GIT_TAG master)

# The magic_enum release tarball has no top-level directory.
if (EXISTS ${magic_enum_SOURCE_DIR}/include/magic_enum/magic_enum.hpp)
    set(PRISM_MAGIC_ENUM_INCLUDE ${magic_enum_SOURCE_DIR}/include)
else()
    set(PRISM_MAGIC_ENUM_INCLUDE ${magic_enum_SOURCE_DIR}/include/magic_enum/..)
endif()

# ------------------------------------------------------------------ library
add_library(prism STATIC)
add_library(Prism::Prism ALIAS prism)

target_sources(prism
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS Source
        FILES Source/Prism/Prism.cppm
    PRIVATE
        ${demangler_SOURCE_DIR}/source/demangler.cpp
)

# Include paths stay public: consumers still need <Prism/Macros.hpp>, and
# fmt/magic_enum headers for what the macros expand to.
target_include_directories(prism PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${fmt_SOURCE_DIR}/include
    ${PRISM_MAGIC_ENUM_INCLUDE}
    ${demangler_SOURCE_DIR}/include
)

# Definitions that change declarations must match between the module and
# every importer, so they're PUBLIC.
target_compile_definitions(prism
    PUBLIC
        FMT_STATIC_THOUSANDS_SEPARATOR=1
        FMT_USE_LONG_DOUBLE=0
        FMT_USE_DOUBLE=0
        FMT_USE_FLOAT=0
        FMT_HEADER_ONLY=1
        MAGIC_ENUM_RANGE_MAX=0x8a00
        PRISM_LOG_ENABLE=1
        PRISM_DISABLE_FMT=0
        PRISM_USE_NAMESPACE=1
    PRIVATE
        PRISM_MODULE_BUILD=1
)

set(PRISM_WARNINGS -Wall -Wextra -Wno-unused-parameter -Wno-frame-address)
if (PRISM_WERROR)
    list(APPEND PRISM_WARNINGS -Werror)
endif()
target_compile_options(prism PRIVATE ${PRISM_WARNINGS})
target_compile_options(prism PUBLIC -fno-omit-frame-pointer -fsized-deallocation)

# std::stacktrace lives in a separate library on GCC.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(prism PUBLIC stdc++exp)
endif()

# -------------------------------------------------------------------- tests
if (PRISM_BUILD_TESTS)
    enable_testing()

    # Like meson: gtest_main is linked into every test. Tests with their own
    # main() never pull its main() out of the static library.
    find_package(GTest QUIET)

    # Same test list as Tests/*/meson.build.
    set(PRISM_TESTS
        Algorithm/Random Algorithm/SearchString Algorithm/Sort
        Core/TypeTraits
        Containers/BitSpan Containers/IntrusiveList Containers/IntrusiveListV3
        Containers/IntrusiveRefList Containers/IntrusiveRedBlackTree
        Containers/DoublyLinkedList Containers/Queue Containers/RingBuffer
        Containers/RedBlackTree Containers/Stack Containers/Tuple
        Containers/UnorderedMap Containers/Vector
        Debug/Log
        Math/Matrix Math/Vector
        Memory/Memory Memory/Ref Memory/Scope Memory/SlabAllocator
        Memory/WeakRef
        String/BasicString String/BasicStringView String/StringBuilder
        String/StringUtils
        Utility/Atomic Utility/AtomicBuiltins Utility/Delegate Utility/Path
        Utility/SimdIntrinsics
    )

    foreach (test ${PRISM_TESTS})
        if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Tests/${test}/main.cpp)
            continue()
        endif()
        string(REPLACE "/" "_" target "test_${test}")
        add_executable(${target} Tests/${test}/main.cpp)
        target_link_libraries(${target} PRIVATE prism)
        if (GTest_FOUND)
            target_link_libraries(${target} PRIVATE GTest::gtest_main)
        endif()
        # Switches the tests from #include to `import Prism;`.
        target_compile_definitions(${target} PRIVATE PRISM_TEST_MODULES=1)
        target_compile_options(${target} PRIVATE ${PRISM_WARNINGS})
        add_test(NAME ${test} COMMAND ${target})
    endforeach()
endif()
