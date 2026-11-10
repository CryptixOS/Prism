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
  -DPRISM_DISABLE_FMT=0
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
target_include_directories(prism_test_support PUBLIC
  "${PROJECT_SOURCE_DIR}/Source" "${PROJECT_BINARY_DIR}")
target_compile_options(prism_test_support PUBLIC
  ${PRISM_CXX_ARGS} ${PRISM_WARNING_ARGS} ${PRISM_TEST_CXX_ARGS})
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
prism_add_tests(Algorithm Random SearchString Sort)
prism_add_tests(Containers
  BitSpan Deque DoublyLinkedList IntrusiveList IntrusiveListV3
  IntrusiveRedBlackTree IntrusiveRefList Queue RedBlackTree RingBuffer Stack
  Tuple UnorderedMap Vector)
prism_add_tests(Core TypeTraits)
prism_add_tests(Debug Log)
# Same test with Logger taking Prism::FormatString instead of fmt's.
add_executable(test_Debug_LogPrismFormat "${CMAKE_CURRENT_SOURCE_DIR}/Debug/Log/main.cpp")
set_target_properties(test_Debug_LogPrismFormat PROPERTIES
  OUTPUT_NAME LogPrismFormat
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Debug")
target_compile_definitions(test_Debug_LogPrismFormat PRIVATE PRISM_LOGGER_USE_PRISM_FORMAT=1)
target_link_libraries(test_Debug_LogPrismFormat PRIVATE ${PRISM_TEST_LINK_LIBS})
add_test(NAME "Debug/LogPrismFormat" COMMAND test_Debug_LogPrismFormat)
set_tests_properties("Debug/LogPrismFormat" PROPERTIES LABELS Debug TIMEOUT 30)
prism_add_tests(Math Matrix Vector)
prism_add_tests(Memory Memory Ref Scope SlabAllocator WeakRef)
prism_add_tests(String BasicString BasicStringView Format StringBuilder StringUtils)
prism_add_tests(Test Framework)
prism_add_tests(Utility Atomic AtomicBuiltins Delegate Mutex Path SimdIntrinsics)
