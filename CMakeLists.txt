#*
#* Created by v1tr10l7 on 07.11.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/
# Prism's primary build system. meson.build is deprecated and only kept for
# backward compatibility. Typical use:
#
#   cmake -S . -B build -G Ninja -DPRISM_TARGET=app -DPRISM_BUILD_TESTS=ON
#   cmake --build build && ctest --test-dir build
#
# From another CMake project, either add_subdirectory(Prism) or, after
# installing, find_package(Prism); then link Prism::Prism (and Prism::Test /
# Prism::TestMain for unit tests).
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
option(PRISM_BUILD_BENCHMARKS "Build the benchmarks (hosted targets only)" OFF)
option(PRISM_WERROR "Treat warnings as errors (meson: werror=true)" ON)

# The patched fmt 10.2 (fmt-10.2.0.patch) is only needed on CryptixOS; host
# builds (target=app) use the host's fmt by default.
set(PRISM_SYSTEM_FMT_DEFAULT OFF)
if(PRISM_TARGET STREQUAL "app")
  set(PRISM_SYSTEM_FMT_DEFAULT ON)
endif()
option(PRISM_USE_SYSTEM_FMT
  "Use the host's fmt instead of the bundled, patched fmt 10.2"
  ${PRISM_SYSTEM_FMT_DEFAULT})

set(PRISM_PKGCONFIG_NAME "prism" CACHE STRING "Name of the installed pkg-config file")

set(PRISM_HOSTED OFF)
if(PRISM_TARGET STREQUAL "app" OR PRISM_TARGET STREQUAL "cryptix-app")
  set(PRISM_HOSTED ON)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
# Prism/Utility is made of C++20 named modules (Source/Prism/Utility/*.ixx).
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
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
# Bundled dependencies come from the meson subprojects when they're already
# downloaded, otherwise they're fetched (fmt gets fmt-10.2.0.patch applied).
include(FetchContent)
set(PRISM_SUBPROJECTS "${CMAKE_CURRENT_SOURCE_DIR}/subprojects")

if(PRISM_USE_SYSTEM_FMT)
  find_package(fmt REQUIRED)
  set(PRISM_FMT_DEFINES "")
else()
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
  # Configuration the patched, freestanding fmt is built for.
  set(PRISM_FMT_DEFINES
    FMT_STATIC_THOUSANDS_SEPARATOR=1
    FMT_USE_LONG_DOUBLE=0
    FMT_USE_DOUBLE=0
    FMT_USE_FLOAT=0
    FMT_USE_FLOAT128=0
    FMT_USE_EXCEPTIONS=0
    FMT_HEADER_ONLY=1
  )
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
  "$<$<BOOL:${PRISM_FMT_DIR}>:${PRISM_FMT_DIR}/include>"
  "${PRISM_MAGIC_ENUM_DIR}/include"
  "${PRISM_DEMANGLER_DIR}/include"
)
if(PRISM_USE_SYSTEM_FMT)
  target_link_libraries(prism_deps INTERFACE fmt::fmt)
endif()

# ==============================================================================
### Flags (same as meson.build's cxx_args)
#
# PRISM_DEFINES are part of Prism's interface and are exported to installed
# consumers (CMake package and pkg-config); PRISM_CODEGEN_ARGS are only
# propagated within the build tree.
set(PRISM_DEFINES
  ${PRISM_FMT_DEFINES}
  MAGIC_ENUM_RANGE_MAX=0x8a00
  PRISM_LOG_ENABLE=true
)
if(PRISM_TARGET STREQUAL "cryptix" OR PRISM_TARGET STREQUAL "carbonc")
  list(APPEND PRISM_DEFINES PRISM_USE_NAMESPACE)
  if(PRISM_TARGET STREQUAL "cryptix")
    list(APPEND PRISM_DEFINES PRISM_TARGET_CRYPTIX)
  else()
    list(APPEND PRISM_DEFINES PRISM_TARGET_CARBONC)
  endif()
endif()

set(PRISM_CODEGEN_ARGS
  -fno-lto
  -fsized-deallocation
  -Wno-unused-parameter
  -Wno-frame-address
  -fno-omit-frame-pointer
  -fPIE
  -fPIC
  -fno-rtti
  -fno-exceptions
)
if(PRISM_EXTRA_INCS)
  list(APPEND PRISM_CODEGEN_ARGS "-I${PRISM_EXTRA_INCS}")
endif()

# Everything together, as used by Tests/CMakeLists.txt.
list(TRANSFORM PRISM_DEFINES PREPEND "-D" OUTPUT_VARIABLE PRISM_DEFINE_ARGS)
set(PRISM_CXX_ARGS ${PRISM_CODEGEN_ARGS} ${PRISM_DEFINE_ARGS})

# -Wmissing-declarations -Wundef)
set(PRISM_CORE_WARNING_ARGS -Wall -Wextra -Wnon-virtual-dtor -Wsuggest-override -Wcast-align -Wnull-dereference -Wimplicit-fallthrough)
set(PRISM_WARNING_ARGS ${PRISM_CORE_WARNING_ARGS} -Wold-style-cast -Wcast-align -Wnull-dereference -Wdouble-promotion)
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
include(GNUInstallDirs)
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
# Module interface units, one named module per file (Prism.<Dir>.<Name>).
# Projects that compile Prism from source (CryptixOS) add these to their own
# FILE_SET CXX_MODULES. Variant.ixx is left out: it doesn't compile yet.
set(PRISM_MODULE_INTERFACES
  Source/Prism/Algorithm/FNV1aHash.ixx
  Source/Prism/Algorithm/Find.ixx
  Source/Prism/Algorithm/Hash.ixx
  Source/Prism/Algorithm/Random.ixx
  Source/Prism/Algorithm/SearchString.ixx
  Source/Prism/Algorithm/Sort.ixx
  Source/Prism/Containers/Array.ixx
  Source/Prism/Containers/BitSpan.ixx
  Source/Prism/Containers/Bitmap.ixx
  Source/Prism/Containers/CircularQueue.ixx
  Source/Prism/Containers/Deque.ixx
  Source/Prism/Containers/DoublyLinkedList.ixx
  Source/Prism/Containers/IntervalTree.ixx
  Source/Prism/Containers/IntrusiveList.ixx
  Source/Prism/Containers/IntrusiveListV3.ixx
  Source/Prism/Containers/IntrusiveRedBlackTree.ixx
  Source/Prism/Containers/IntrusiveRefList.ixx
  Source/Prism/Containers/KeyValuePair.ixx
  Source/Prism/Containers/Queue.ixx
  Source/Prism/Containers/RadixTree.ixx
  Source/Prism/Containers/RedBlackTree.ixx
  Source/Prism/Containers/RingBuffer.ixx
  Source/Prism/Containers/Set.ixx
  Source/Prism/Containers/Span.ixx
  Source/Prism/Containers/SplitBuffer.ixx
  Source/Prism/Containers/Stack.ixx
  Source/Prism/Containers/Tuple.ixx
  Source/Prism/Containers/UnorderedMap.ixx
  Source/Prism/Containers/Vector.ixx
  Source/Prism/Core/AllocatorTraits.ixx
  Source/Prism/Core/Bits.ixx
  Source/Prism/Core/Concepts.ixx
  Source/Prism/Core/Core.ixx
  Source/Prism/Core/Error.ixx
  Source/Prism/Core/Eternal.ixx
  Source/Prism/Core/HashTraits.ixx
  Source/Prism/Core/Integer128.ixx
  Source/Prism/Core/Iterator.ixx
  Source/Prism/Core/Limits.ixx
  Source/Prism/Core/NonCopyable.ixx
  Source/Prism/Core/NonMovable.ixx
  Source/Prism/Core/ParameterPack.ixx
  Source/Prism/Core/Ranges.ixx
  Source/Prism/Core/Singleton.ixx
  Source/Prism/Core/TypeTraits.ixx
  Source/Prism/Core/TypeTraitsForward.ixx
  Source/Prism/Core/UUID.ixx
  Source/Prism/Debug/Assertions.ixx
  Source/Prism/Debug/Log.ixx
  Source/Prism/Debug/LogSink.ixx
  Source/Prism/Debug/Logger.ixx
  Source/Prism/Debug/SourceLocation.ixx
  Source/Prism/Debug/Stacktrace.ixx
  Source/Prism/Math/Matrix.ixx
  Source/Prism/Math/SimdVector.ixx
  Source/Prism/Math/Vector.ixx
  Source/Prism/Memory/Allocator.ixx
  Source/Prism/Memory/BitStream.ixx
  Source/Prism/Memory/Buffer.ixx
  Source/Prism/Memory/BumpAllocator.ixx
  Source/Prism/Memory/ByteStream.ixx
  Source/Prism/Memory/Endian.ixx
  Source/Prism/Memory/Memory.ixx
  Source/Prism/Memory/Pointer.ixx
  Source/Prism/Memory/Ref.ixx
  Source/Prism/Memory/RefCounted.ixx
  Source/Prism/Memory/SSOStorage.ixx
  Source/Prism/Memory/Scope.ixx
  Source/Prism/Memory/SlabAllocator.ixx
  Source/Prism/Memory/SlabPool.ixx
  Source/Prism/Memory/SlobAllocator.ixx
  Source/Prism/Memory/WeakRef.ixx
  Source/Prism/Network/IPv4Address.ixx
  Source/Prism/Ranges/Range.ixx
  Source/Prism/Ranges/Ranges.ixx
  Source/Prism/Ranges/Views.ixx
  Source/Prism/String/CharTraits.ixx
  Source/Prism/String/CodePoints.ixx
  Source/Prism/String/FormatHandler.ixx
  Source/Prism/String/FormatString.ixx
  Source/Prism/String/Formatter.ixx
  Source/Prism/String/FormatterContext.ixx
  Source/Prism/String/Printf.ixx
  Source/Prism/String/String.ixx
  Source/Prism/String/StringBuilder.ixx
  Source/Prism/String/StringUtils.ixx
  Source/Prism/String/StringView.ixx
  Source/Prism/Utility/Atomic.ixx
  Source/Prism/Utility/AtomicBuiltins.ixx
  Source/Prism/Utility/Checksum.ixx
  Source/Prism/Utility/Compare.ixx
  Source/Prism/Utility/Delegate.ixx
  Source/Prism/Utility/Enum.ixx
  Source/Prism/Utility/Expected.ixx
  Source/Prism/Utility/Hash.ixx
  Source/Prism/Utility/LockingPolicy.ixx
  Source/Prism/Utility/Math.ixx
  Source/Prism/Utility/Mutex.ixx
  Source/Prism/Utility/OptionParser.ixx
  Source/Prism/Utility/Optional.ixx
  Source/Prism/Utility/Path.ixx
  Source/Prism/Utility/PathView.ixx
  Source/Prism/Utility/RangeAllocator.ixx
  Source/Prism/Utility/SimdIntrinsics.ixx
  Source/Prism/Utility/SimdVector.ixx
  Source/Prism/Utility/Time.ixx
)
# SIMD is off limits in the kernel (SimdIntrinsics.ixx #errors there).
if(PRISM_TARGET STREQUAL "cryptix")
  list(REMOVE_ITEM PRISM_MODULE_INTERFACES
    Source/Prism/Math/SimdVector.ixx
    Source/Prism/Utility/SimdIntrinsics.ixx)
endif()
# The test framework's modules (Prism.Test.*), built into prism_test.
set(PRISM_TEST_MODULE_INTERFACES
  Source/Prism/Test/CrashTest.ixx
  Source/Prism/Test/Macros.ixx
  Source/Prism/Test/Randomized/Generator.ixx
  Source/Prism/Test/Randomized/RandSource.ixx
  Source/Prism/Test/Randomized/RandomRun.ixx
  Source/Prism/Test/Test.ixx
  Source/Prism/Test/TestCase.ixx
  Source/Prism/Test/TestResult.ixx
  Source/Prism/Test/TestSuite.ixx
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
target_sources(prism PUBLIC FILE_SET CXX_MODULES BASE_DIRS Source
  FILES ${PRISM_MODULE_INTERFACES})
add_library(Prism ALIAS prism) # target name of the previous CMakeLists.txt
target_include_directories(prism PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_compile_definitions(prism PUBLIC ${PRISM_DEFINES})
target_compile_options(prism
  PUBLIC "$<BUILD_INTERFACE:${PRISM_CODEGEN_ARGS}>"
  PRIVATE ${PRISM_WARNING_ARGS})
target_link_libraries(prism PUBLIC "$<BUILD_INTERFACE:prism_deps>")
if(PRISM_USE_SYSTEM_FMT)
  target_link_libraries(prism PUBLIC "$<INSTALL_INTERFACE:fmt::fmt>")
endif()

# Unit testing framework (Prism/Test), only for hosted targets.
if(PRISM_HOSTED)
  add_library(prism_test STATIC ${PRISM_TEST_FRAMEWORK_SOURCES})
  add_library(Prism::Test ALIAS prism_test)
  target_link_libraries(prism_test PUBLIC prism)
  target_compile_options(prism_test PRIVATE ${PRISM_WARNING_ARGS})
  target_sources(prism_test PUBLIC FILE_SET CXX_MODULES BASE_DIRS Source
    FILES ${PRISM_TEST_MODULE_INTERFACES})

  add_library(prism_test_main STATIC ${PRISM_TEST_MAIN_SOURCES})
  add_library(Prism::TestMain ALIAS prism_test_main)
  target_link_libraries(prism_test_main PUBLIC prism_test)
  target_compile_options(prism_test_main PRIVATE ${PRISM_WARNING_ARGS})
endif()

# ==============================================================================
### Install (headers, libraries, CMake package, pkg-config)
include(CMakePackageConfigHelpers)
set(PRISM_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Prism"
  CACHE STRING "Where to install the CMake package files")
set(PRISM_INSTALL_PKGCONFIGDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
  CACHE STRING "Where to install the pkg-config file")

set_target_properties(prism PROPERTIES EXPORT_NAME Prism)
set(PRISM_INSTALL_TARGETS prism)
if(PRISM_HOSTED)
  set_target_properties(prism_test PROPERTIES EXPORT_NAME Test)
  set_target_properties(prism_test_main PROPERTIES EXPORT_NAME TestMain)
  list(APPEND PRISM_INSTALL_TARGETS prism_test prism_test_main)
endif()
install(TARGETS ${PRISM_INSTALL_TARGETS} EXPORT PrismTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT PrismTargets NAMESPACE Prism::
  DESTINATION ${PRISM_INSTALL_CMAKEDIR})

install(DIRECTORY Source/Prism DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl" PATTERN "*.ixx")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Version.hpp"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Prism/Core)
# Bundled header-only dependencies used by Prism's public headers.
install(DIRECTORY "${PRISM_MAGIC_ENUM_DIR}/include/magic_enum"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
if(NOT PRISM_USE_SYSTEM_FMT)
  install(DIRECTORY "${PRISM_FMT_DIR}/include/fmt"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/PrismConfig.cmake.in" [=[
@PACKAGE_INIT@
include(CMakeFindDependencyMacro)
if(@PRISM_USE_SYSTEM_FMT@)
  find_dependency(fmt)
endif()
include("${CMAKE_CURRENT_LIST_DIR}/PrismTargets.cmake")
check_required_components(Prism)
]=])
configure_package_config_file(
  "${CMAKE_CURRENT_BINARY_DIR}/PrismConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/PrismConfig.cmake"
  INSTALL_DESTINATION ${PRISM_INSTALL_CMAKEDIR})
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/PrismConfigVersion.cmake"
  COMPATIBILITY SameMinorVersion)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/PrismConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/PrismConfigVersion.cmake"
  DESTINATION ${PRISM_INSTALL_CMAKEDIR})

# pkg-config. libdir/includedir are relative to the prefix when possible so
# the file works for any install prefix.
foreach(dir LIBDIR INCLUDEDIR)
  if(IS_ABSOLUTE "${CMAKE_INSTALL_${dir}}")
    set(PRISM_PC_${dir} "${CMAKE_INSTALL_${dir}}")
  else()
    set(PRISM_PC_${dir} "\${prefix}/${CMAKE_INSTALL_${dir}}")
  endif()
endforeach()
string(JOIN " " PRISM_PC_CFLAGS ${PRISM_DEFINE_ARGS})
set(PRISM_PC_REQUIRES "")
if(PRISM_USE_SYSTEM_FMT)
  set(PRISM_PC_REQUIRES "Requires: fmt")
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/prism.pc"
"prefix=${CMAKE_INSTALL_PREFIX}
libdir=${PRISM_PC_LIBDIR}
includedir=${PRISM_PC_INCLUDEDIR}

Name: prism
Description: Freestanding C++23 standard library and runtime for kernel-space development
URL: https://github.com/CryptixOS/Prism
Version: ${PROJECT_VERSION}
${PRISM_PC_REQUIRES}
Libs: -L\${libdir} -lprism
Cflags: -I\${includedir} ${PRISM_PC_CFLAGS}
")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/prism.pc"
  DESTINATION ${PRISM_INSTALL_PKGCONFIGDIR}
  RENAME "${PRISM_PKGCONFIG_NAME}.pc")

# ==============================================================================
### Tests
if(PRISM_BUILD_TESTS)
  if(NOT PRISM_HOSTED)
    message(FATAL_ERROR "PRISM_BUILD_TESTS requires a hosted PRISM_TARGET (app or cryptix-app)")
  endif()
  enable_testing()
  add_subdirectory(Tests)
endif()

# ==============================================================================
### Benchmarks
if(PRISM_BUILD_BENCHMARKS)
  if(NOT PRISM_HOSTED)
    message(FATAL_ERROR "PRISM_BUILD_BENCHMARKS requires a hosted PRISM_TARGET (app or cryptix-app)")
  endif()
  add_subdirectory(Benchmarks)
endif()
