#*
#* Created by v1tr10l7 on 15.07.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

memory_tests = [
  'Memory', 'Ref', 'Scope', 'SlabAllocator', 'WeakRef'
]

foreach name : memory_tests
  test = executable(
    name, files(name / 'main.cpp'),
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incs, dependencies: test_deps
  )
  test(name, test, suite: 'Memory')
endforeach

