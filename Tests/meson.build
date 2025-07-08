#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

incdirs = include_directories('../Source/')
tests = [
  'Algorithm', 'Memory', 'Ref', 
  'SimdIntrinsics', 'TypeTraits', 
]
cpp_args = [
  '-Wno-unused-parameter',
  '-Wno-self-assign-overloaded',
]
link_args = [
  '-lstdc++exp',
]

srcs = [
  meson.project_source_root() / 'Source/Prism/Containers/RingBuffer.cpp',
  meson.project_source_root() / 'Source/Prism/Debug/Assertions.cpp',
  meson.project_source_root() / 'Source/Prism/Debug/Log.cpp',
  meson.project_source_root() / 'Source/Prism/Memory/Memory.cpp',

  meson.project_source_root() / 'Source/Prism/Utility/PathView.cpp',
  meson.project_source_root() / 'Source/Prism/Utility/Path.cpp'
]

subdir('Containers')
subdir('String')

foreach name : tests
  test = executable(
    name, files(srcs, name / 'main.cpp'),
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incdirs, dependencies: deps
  )
  test(name, test)
endforeach
