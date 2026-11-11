#*
#* Created by v1tr10l7 on 02.09.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/

debug_tests = [
  'Log', 
]

foreach name : debug_tests
  test = executable(
    name, files(name / 'main.cpp'),
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incs, dependencies: test_deps
  )
  test(name, test, suite: 'Debug')
endforeach
