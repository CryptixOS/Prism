#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

incdirs = include_directories('../Source/')
tests = [
  'Algorithm', 'IntrusiveList', 'Deque',
  'DoublyLinkedList', 'Ref', 'RingBuffer',
  'RedBlackTree', 'SimdIntrinsics', 'String', 'StringBuilder',
  'StringUtils', #'Tuple',
  'TypeTraits', 'UnorderedMap', 'Vector',
]
cpp_args = [
  '-Wno-unused-parameter',
  '-Wno-self-assign-overloaded',
]
link_args = [
  '-lstdc++exp',
]

foreach name : tests
  srcs = files(
    meson.project_source_root() / 'Source/Prism/Containers/RingBuffer.cpp',
    meson.project_source_root() / 'Source/Prism/Debug/Assertions.cpp',
    meson.project_source_root() / 'Source/Prism/Debug/Log.cpp',
    meson.project_source_root() / 'Source/Prism/Utility/PathView.cpp',
    meson.project_source_root() / 'Source/Prism/Utility/Path.cpp',
    name / 'main.cpp')
  test = executable(
    name, srcs,
    cpp_args: cpp_args, link_args: link_args,
    include_directories: incdirs, dependencies: deps
  )
  test(name, test)
endforeach
