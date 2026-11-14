#*
#* Created by v1tr10l7 on 14.11.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/
# libFuzzer targets, one per Fuzz/<Name>/main.cpp, built as Fuzz/<Name>:
#
#   Format        format strings (Prism::Format with Runtime())
#   Path          PathView / Path operations
#   OptionParser  command lines through getopt / getopt_long(_only)
#   Number        StringUtils::ToNumber / ToString round trips
#   String        StringView / String searches and splitting
#
# Needs clang. The whole build gets -fsanitize=fuzzer-no-link plus ASan and
# UBSan (see PRISM_BUILD_FUZZERS in the top-level CMakeLists.txt). Run one:
#
#   build/Fuzz/Format -max_total_time=60 Fuzz/Format/corpus

function(prism_add_fuzzers)
  foreach(name IN LISTS ARGN)
    set(target "fuzz_${name}")
    add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/${name}/main.cpp")
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME ${name}
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(${target} PRIVATE prism)
    target_link_options(${target} PRIVATE -fsanitize=fuzzer)
  endforeach()
endfunction()

prism_add_fuzzers(Format Path OptionParser Number String)
