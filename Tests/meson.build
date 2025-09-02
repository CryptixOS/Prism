#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

tests = [
  'Ref', 'SimdIntrinsics', 
]
cpp_args = [
  '-Wno-unused-parameter',
  '-Wno-self-assign-overloaded',
  '-lgcc_s',
  '-fmodules-ts',
]
link_args = [
  '-lstdc++exp',
]

subdir('Algorithm')
subdir('Core')
subdir('Containers')
subdir('Debug')
subdir('Memory')
subdir('String')

extraincs = get_option('extra_incs')
if extraincs != ''
  cpp_args += '-I' + extraincs
endif

foreach name : tests
  test = executable(
    name, [srcs, files(name / 'main.cpp')],
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incs, dependencies: deps
  )
  test(name, test)
endforeach
