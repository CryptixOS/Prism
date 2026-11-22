#*
#* Created by v1tr10l7 on 07.11.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/
# Mirrors Tests/meson.build: Prism and the test framework are compiled once
# with the test flags, and each Tests/<Suite>/<Name>/main.cpp becomes one
# executable registered with CTest (label = suite).

set(PRISM_TEST_CXX_ARGS
  -Wno-unused-parameter
  -DPRISM_USE_NAMESPACE=1
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND PRISM_TEST_CXX_ARGS -Wno-self-assign-overloaded)
endif()

add_library(prism_test_support STATIC
  ${PRISM_SOURCES}
  "${PRISM_DEMANGLER_DIR}/source/demangler.cpp"
  ${PRISM_TEST_FRAMEWORK_SOURCES}
)
list(TRANSFORM PRISM_SOURCES PREPEND "${PROJECT_SOURCE_DIR}/"
  OUTPUT_VARIABLE PRISM_SOURCES_ABS)
list(TRANSFORM PRISM_TEST_FRAMEWORK_SOURCES PREPEND "${PROJECT_SOURCE_DIR}/"
  OUTPUT_VARIABLE PRISM_TEST_FRAMEWORK_SOURCES_ABS)
set_target_properties(prism_test_support PROPERTIES SOURCES
  "${PRISM_SOURCES_ABS};${PRISM_DEMANGLER_DIR}/source/demangler.cpp;${PRISM_TEST_FRAMEWORK_SOURCES_ABS}")
# Its own copy of the modules: a BMI keeps the macros it was compiled with,
# and the tests build with PRISM_USE_NAMESPACE=1.
list(TRANSFORM PRISM_MODULE_INTERFACES PREPEND "${PROJECT_SOURCE_DIR}/"
  OUTPUT_VARIABLE PRISM_MODULE_INTERFACES_ABS)
list(TRANSFORM PRISM_TEST_MODULE_INTERFACES PREPEND "${PROJECT_SOURCE_DIR}/"
  OUTPUT_VARIABLE PRISM_TEST_MODULE_INTERFACES_ABS)
target_sources(prism_test_support PUBLIC FILE_SET CXX_MODULES
  BASE_DIRS "${PROJECT_SOURCE_DIR}/Source" "${PRISM_GENERATED_DIR}"
  FILES ${PRISM_MODULE_INTERFACES_ABS} ${PRISM_GENERATED_MODULE_INTERFACES}
  ${PRISM_TEST_MODULE_INTERFACES_ABS})
target_include_directories(prism_test_support PUBLIC
  "${PROJECT_SOURCE_DIR}/Source" "${PROJECT_BINARY_DIR}")
target_compile_options(prism_test_support PUBLIC
  ${PRISM_CXX_ARGS} ${PRISM_CORE_WARNING_ARGS} ${PRISM_TEST_CXX_ARGS})
target_link_libraries(prism_test_support PUBLIC prism_deps)

add_library(prism_test_support_main STATIC
  "${PROJECT_SOURCE_DIR}/${PRISM_TEST_MAIN_SOURCES}")
target_link_libraries(prism_test_support_main PUBLIC prism_test_support)

set(PRISM_TEST_LINK_LIBS prism_test_support_main prism_test_support)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  list(APPEND PRISM_TEST_LINK_LIBS stdc++exp)
endif()

function(prism_add_tests suite)
  foreach(name IN LISTS ARGN)
    set(target "test_${suite}_${name}")
    add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/${suite}/${name}/main.cpp")
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME ${name}
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${suite}")
    target_link_libraries(${target} PRIVATE ${PRISM_TEST_LINK_LIBS})
    add_test(NAME "${suite}/${name}" COMMAND ${target})
    set_tests_properties("${suite}/${name}" PROPERTIES LABELS ${suite} TIMEOUT 30)
  endforeach()
endfunction()

# Keep these lists in sync with Tests/<Suite>/meson.build.
prism_add_tests(Algorithm Find Random SearchString SipHash Sort)
prism_add_tests(Containers
  BitSpan Bitmap Deque DoublyLinkedList EdgeCases IntrusiveList
  IntrusiveRedBlackTree Queue RedBlackTree RingBuffer Stack
  Tuple UnorderedMap Vector)
prism_add_tests(Core Checked Hardening NumericLimits TypeTraits Version)
prism_add_tests(Debug Log)
prism_add_tests(Math Matrix Vector)
prism_add_tests(Memory Memory Ref Scope SlabAllocator WeakRef)
prism_add_tests(Ranges Views)
prism_add_tests(String BasicString BasicStringView Format FormatGolden StringBuilder
  StringUtils)
prism_add_tests(Test Framework)
prism_add_tests(Utility Atomic AtomicBuiltins Delegate Enum Mutex OptionParser Path
  SimdIntrinsics Variant)

# Every public header compiles on its own: one translation unit per header,
# including only that header. (Modules are compiled on their own anyway.)
file(GLOB_RECURSE PRISM_PUBLIC_HEADERS RELATIVE "${PROJECT_SOURCE_DIR}/Source"
  CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/Source/Prism/*.hpp")
set(PRISM_HEADER_CHECK_SOURCES "")
foreach(header IN LISTS PRISM_PUBLIC_HEADERS)
  string(MAKE_C_IDENTIFIER "${header}" id)
  set(source "${CMAKE_CURRENT_BINARY_DIR}/HeaderCheck/${id}.cpp")
  file(CONFIGURE OUTPUT "${source}" CONTENT "#include <${header}>\n")
  list(APPEND PRISM_HEADER_CHECK_SOURCES "${source}")
endforeach()
add_library(prism_header_check OBJECT ${PRISM_HEADER_CHECK_SOURCES})
target_include_directories(prism_header_check PRIVATE
  "${PROJECT_SOURCE_DIR}/Source" "${PROJECT_BINARY_DIR}")
target_compile_options(prism_header_check PRIVATE
  ${PRISM_CXX_ARGS} ${PRISM_WARNING_ARGS})
set_target_properties(prism_header_check PROPERTIES CXX_SCAN_FOR_MODULES OFF)

# A format string that doesn't match its arguments must not compile, and the
# error must name the problem.
add_library(prism_format_compile_fail OBJECT EXCLUDE_FROM_ALL
  "${CMAKE_CURRENT_SOURCE_DIR}/String/FormatCompileFail/main.cpp")
target_link_libraries(prism_format_compile_fail PRIVATE prism_test_support)
add_test(NAME String/FormatCompileFail
  COMMAND "${CMAKE_COMMAND}" --build "${PROJECT_BINARY_DIR}"
          --target prism_format_compile_fail)
set_tests_properties(String/FormatCompileFail PROPERTIES LABELS String
  TIMEOUT 120 PASS_REGULAR_EXPRESSION "argument not found")
