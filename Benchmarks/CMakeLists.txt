#*
#* Created by v1tr10l7 on 13.11.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/
# Each Benchmarks/<Name>/main.cpp becomes the executable Benchmarks/<Name>:
#
#   Sort        Prism::Sort & co. vs std::sort, many input patterns
#   Containers  Vector, Deque, Queue, Stack, lists, RedBlackTree, Set,
#               UnorderedMap, Bitmap vs their std counterparts
#   Memory      Memory::Copy/Fill/Move/Compare/Scan vs libc; Ref, WeakRef,
#               Scope vs <memory>; slab/bump allocators vs malloc
#   String      String, StringView, StringUtils, StringBuilder, Format vs
#               std::string, <charconv>, std::format
#   Utility     Find, hashes, checksums, Random, Optional, Expected,
#               Delegate, Atomic, locks, Path, Math, Ranges
#
# All but Sort share Common/Bench.hpp (flags: --filter, --min-time-ms,
# --reps, --csv, --list, --quick). They measure nothing useful without
# optimizations, so configure with -DCMAKE_BUILD_TYPE=Release.

if(NOT CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  message(WARNING "Benchmarks are being built without optimizations "
    "(CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}); use Release for real numbers")
endif()

function(prism_add_benchmarks)
  foreach(name IN LISTS ARGN)
    set(target "bench_${name}")
    add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/${name}/main.cpp")
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME ${name}
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(${target} PRIVATE prism)
  endforeach()
endfunction()

prism_add_benchmarks(Sort Containers Memory String Utility)
