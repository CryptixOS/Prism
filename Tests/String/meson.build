#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

string_tests = [
  'BasicString', 'BasicStringView', 'StringBuilder',
  'StringUtils',
]

foreach name : string_tests
  test = executable(
    name, files(srcs, name / 'main.cpp'),
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incdirs, dependencies: deps
  )
  test(name, test, suite: 'String')
endforeach
