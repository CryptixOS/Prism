#*
#* Created by v1tr10l7 on 07.11.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/
# CMake build, kept equivalent to meson.build (same sources, flags, options
# and dependencies). Typical use:
#
#   cmake -S . -B build-cmake -G Ninja -DPRISM_BUILD_TESTS=ON
#   cmake --build build-cmake && ctest --test-dir build-cmake
#
# or, from another CMake project: add_subdirectory(Prism) and link
# Prism::Prism (and Prism::Test / Prism::TestMain for unit tests).
cmake_minimum_required(VERSION 3.24)

project(prism VERSION 0.0.1 LANGUAGES C CXX)

set(PRISM_TARGETS cryptix-app app cryptix carbonc)
set(PRISM_TARGET "cryptix-app" CACHE STRING "Target (${PRISM_TARGETS})")
set_property(CACHE PRISM_TARGET PROPERTY STRINGS ${PRISM_TARGETS})
if(NOT PRISM_TARGET IN_LIST PRISM_TARGETS)
  message(FATAL_ERROR "PRISM_TARGET must be one of: ${PRISM_TARGETS}")
endif()
set(PRISM_EXTRA_INCS "" CACHE STRING "Extra include directory")
option(PRISM_BUILD_TESTS "Build the unit tests" OFF)
option(PRISM_WERROR "Treat warnings as errors (meson: werror=true)" ON)

set(PRISM_HOSTED OFF)
if(PRISM_TARGET STREQUAL "app" OR PRISM_TARGET STREQUAL "cryptix-app")
  set(PRISM_HOSTED ON)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=gnu++23 PRISM_HAS_CXX23)
if(NOT PRISM_HAS_CXX23)
  message(FATAL_ERROR "C++23 support is needed. Please install clang 17 or higher.")
endif()

# ==============================================================================
### Dependencies
#
# Uses the meson subprojects when they're already downloaded, otherwise
# fetches the same sources (fmt gets fmt-10.2.0.patch applied).
include(FetchContent)
set(PRISM_SUBPROJECTS "${CMAKE_CURRENT_SOURCE_DIR}/subprojects")

if(EXISTS "${PRISM_SUBPROJECTS}/fmt-10.2.0/include/fmt/format.h")
  set(PRISM_FMT_DIR "${PRISM_SUBPROJECTS}/fmt-10.2.0")
else()
  find_program(PRISM_PATCH patch REQUIRED)
  FetchContent_Declare(prism_fmt
    URL https://github.com/fmtlib/fmt/archive/10.2.0.tar.gz
    URL_HASH SHA256=3ca91733a7313a8ad41c0885929415f8ec0a2a31d4dc7e27e9331412f4ca26ac
    PATCH_COMMAND ${PRISM_PATCH} -p1 -E --forward
                  -i "${CMAKE_CURRENT_SOURCE_DIR}/fmt-10.2.0.patch"
    SOURCE_SUBDIR prism-no-cmake # header-only: don't add fmt's own project
  )
  FetchContent_MakeAvailable(prism_fmt)
  set(PRISM_FMT_DIR "${prism_fmt_SOURCE_DIR}")
endif()

if(EXISTS "${PRISM_SUBPROJECTS}/magic_enum/include")
  set(PRISM_MAGIC_ENUM_DIR "${PRISM_SUBPROJECTS}/magic_enum")
else()
  FetchContent_Declare(prism_magic_enum
    URL https://github.com/Neargye/magic_enum/releases/download/v0.9.5/magic_enum-v0.9.5.tar.gz
    URL_HASH SHA256=0694e1ef160cbb6b8be2f43c7df349295911cb7daebdd4e557adaac2c18420f9
    SOURCE_SUBDIR prism-no-cmake
  )
  FetchContent_MakeAvailable(prism_magic_enum)
  set(PRISM_MAGIC_ENUM_DIR "${prism_magic_enum_SOURCE_DIR}")
endif()

if(EXISTS "${PRISM_SUBPROJECTS}/demangler/source/demangler.cpp")
  set(PRISM_DEMANGLER_DIR "${PRISM_SUBPROJECTS}/demangler")
else()
  FetchContent_Declare(prism_demangler
    GIT_REPOSITORY https://github.com/ilobilo/demangler
    GIT_TAG master
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR prism-no-cmake
  )
  FetchContent_MakeAvailable(prism_demangler)
  set(PRISM_DEMANGLER_DIR "${prism_demangler_SOURCE_DIR}")
endif()

add_library(prism_deps INTERFACE)
target_include_directories(prism_deps SYSTEM INTERFACE
  "$<BUILD_INTERFACE:${PRISM_FMT_DIR}/include>"
  "$<BUILD_INTERFACE:${PRISM_MAGIC_ENUM_DIR}/include>"
  "$<BUILD_INTERFACE:${PRISM_DEMANGLER_DIR}/include>"
)
# fmt's own meson.build exports these with fmt_dep.
target_compile_definitions(prism_deps INTERFACE
  FMT_STATIC_THOUSANDS_SEPARATOR=1
  FMT_USE_LONG_DOUBLE=0
  FMT_USE_DOUBLE=0
  FMT_USE_FLOAT=0
  FMT_HEADER_ONLY
)

# ==============================================================================
### Flags (same as meson.build's cxx_args)
set(PRISM_CXX_ARGS
  -fno-lto
  -fsized-deallocation
  -Wno-unused-parameter
  -Wno-frame-address
  -DFMT_STATIC_THOUSANDS_SEPARATOR=1
  -DFMT_USE_LONG_DOUBLE=0
  -DFMT_USE_DOUBLE=0
  -DFMT_USE_FLOAT=0
  -DFMT_USE_FLOAT128=0
  -DFMT_USE_EXCEPTIONS=0
  -DFMT_HEADER_ONLY=1
  -DMAGIC_ENUM_RANGE_MAX=0x8a00
  -DPRISM_LOG_ENABLE=true
  -fno-omit-frame-pointer
  -fPIE
  -fPIC
  -fno-rtti
  -fno-exceptions
)
if(PRISM_TARGET STREQUAL "cryptix" OR PRISM_TARGET STREQUAL "carbonc")
  list(APPEND PRISM_CXX_ARGS -DPRISM_USE_NAMESPACE)
  if(PRISM_TARGET STREQUAL "cryptix")
    list(APPEND PRISM_CXX_ARGS -DPRISM_TARGET_CRYPTIX)
  else()
    list(APPEND PRISM_CXX_ARGS -DPRISM_TARGET_CARBONC)
  endif()
endif()
if(PRISM_EXTRA_INCS)
  list(APPEND PRISM_CXX_ARGS "-I${PRISM_EXTRA_INCS}")
endif()

set(PRISM_WARNING_ARGS -Wall -Wextra)
if(PRISM_WERROR)
  list(APPEND PRISM_WARNING_ARGS -Werror)
endif()

# ==============================================================================
### Version.hpp (same values as meson's configuration_data)
execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE git_tag OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
set(library_name "prism")
set(PM_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(PM_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(PM_VERSION_PATCH ${PROJECT_VERSION_PATCH})
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch)
if(arch STREQUAL "amd64")
  set(arch "x86_64")
endif()
string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" compiler)
if(compiler STREQUAL "gnu")
  set(compiler "gcc")
endif()
set(compiler_version "${CMAKE_CXX_COMPILER_VERSION}")
set(library_version_string
  "${PM_VERSION_MAJOR}.${PM_VERSION_MINOR}.${PM_VERSION_PATCH}-${git_tag}")

# The template uses meson's `#mesondefine`; turn it into a plain #define.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Source/Prism/Core/Version.hpp.in"
  PRISM_VERSION_TEMPLATE)
string(REGEX REPLACE "#mesondefine ([A-Za-z0-9_]+)" "#define \\1 @\\1@"
  PRISM_VERSION_TEMPLATE "${PRISM_VERSION_TEMPLATE}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Version.hpp.cmake.in"
  "${PRISM_VERSION_TEMPLATE}")
configure_file("${CMAKE_CURRENT_BINARY_DIR}/Version.hpp.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Version.hpp" @ONLY)

# ==============================================================================
### Library
set(PRISM_SOURCES
  Source/Prism/Algorithm/Hash.cpp
  Source/Prism/Containers/Bitmap.cpp
  Source/Prism/Containers/RingBuffer.cpp
  Source/Prism/Debug/Assertions.cpp
  Source/Prism/Debug/Log.cpp
  Source/Prism/Debug/Logger.cpp
  Source/Prism/Debug/LogSink.cpp
  Source/Prism/Debug/Ubsan.cpp
  Source/Prism/Debug/Stacktrace.cpp
  Source/Prism/Memory/Memory.cpp
  Source/Prism/String/Printf.cpp
  Source/Prism/Utility/Path.cpp
  Source/Prism/Utility/PathView.cpp
  Source/Prism/Utility/Time.cpp
)
set(PRISM_TEST_FRAMEWORK_SOURCES
  Source/Prism/Test/CrashTest.cpp
  Source/Prism/Test/TestSuite.cpp
)
set(PRISM_TEST_MAIN_SOURCES Source/Prism/Test/TestMain.cpp)

add_library(prism STATIC
  ${PRISM_SOURCES}
  "${PRISM_DEMANGLER_DIR}/source/demangler.cpp"
)
add_library(Prism::Prism ALIAS prism)
add_library(Prism ALIAS prism) # target name of the previous CMakeLists.txt
target_include_directories(prism PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<INSTALL_INTERFACE:include>"
)
target_compile_options(prism PUBLIC ${PRISM_CXX_ARGS}
  PRIVATE ${PRISM_WARNING_ARGS})
target_link_libraries(prism PUBLIC "$<BUILD_INTERFACE:prism_deps>")

# Unit testing framework (Prism/Test), only for hosted targets.
if(PRISM_HOSTED)
  add_library(prism_test STATIC ${PRISM_TEST_FRAMEWORK_SOURCES})
  add_library(Prism::Test ALIAS prism_test)
  target_link_libraries(prism_test PUBLIC prism)
  target_compile_options(prism_test PRIVATE ${PRISM_WARNING_ARGS})

  add_library(prism_test_main STATIC ${PRISM_TEST_MAIN_SOURCES})
  add_library(Prism::TestMain ALIAS prism_test_main)
  target_link_libraries(prism_test_main PUBLIC prism_test)
  target_compile_options(prism_test_main PRIVATE ${PRISM_WARNING_ARGS})
endif()

# ==============================================================================
### Install (headers, libraries, pkg-config)
include(GNUInstallDirs)
install(TARGETS prism ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
if(PRISM_HOSTED)
  install(TARGETS prism_test prism_test_main
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
install(DIRECTORY Source/Prism DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Version.hpp"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Prism/Core)

set(PRISM_PC_LIBS "-lprism")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/prism.pc" CONTENT
"prefix=${CMAKE_INSTALL_PREFIX}
libdir=\${prefix}/${CMAKE_INSTALL_LIBDIR}
includedir=\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}

Name: prism
Description: Prism support library
Version: ${PROJECT_VERSION}
Libs: -L\${libdir} ${PRISM_PC_LIBS}
Cflags: -I\${includedir}
")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/prism.pc"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# ==============================================================================
### Tests
if(PRISM_BUILD_TESTS)
  if(NOT PRISM_HOSTED)
    message(FATAL_ERROR "PRISM_BUILD_TESTS requires a hosted PRISM_TARGET (app or cryptix-app)")
  endif()
  enable_testing()
  add_subdirectory(Tests)
endif()
