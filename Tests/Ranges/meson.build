#*
#* Created by v1tr10l7 on 09.11.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/

ranges_tests = [
  'Views',
]

foreach name : ranges_tests
  test = executable(
    name, files(name / 'main.cpp'),
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incs, dependencies: test_deps
  )
  test(name, test, suite: 'Ranges')
endforeach

