#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

incdirs = include_directories('../Source/')
tests = ['DoublyLinkedList', 'RedBlackTree', 'String', 'StringUtils', 'Vector']

foreach name : tests
  srcs = files(
    meson.project_source_root() / 'Source/Prism/Debug/Log.cpp',
    name / 'main.cpp')
  test = executable(name, srcs, include_directories: incdirs, dependencies: deps)
  test(name, test)
endforeach
