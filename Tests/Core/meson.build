#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

core_tests = [
  # 'NumericLimits', 
  'TypeTraits', 
]

foreach name : core_tests
  test = executable(
    name, [srcs, files(name / 'main.cpp')],
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incs, dependencies: deps
  )
  test(name, test, suite: 'Core')
endforeach
