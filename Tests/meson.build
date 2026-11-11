#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/
cpp_args = [
  '-Wno-unused-parameter',
  '-Wno-self-assign-overloaded',
  '-lgcc_s',
  '-fmodules-ts',
  '-DPRISM_USE_NAMESPACE=1',
]
link_args = [
  '-lstdc++exp',
]

extraincs = get_option('extra_incs')
if extraincs != ''
  cpp_args += '-I' + extraincs
endif

# Prism sources + the test framework are compiled once, with the test flags,
# and every test executable links against them (main() comes from
# prism_test_main, see Source/Prism/Test/TestMain.cpp).
prism_test_support = static_library(
  'prism_test_support', [srcs, test_framework_srcs],
  cpp_args: cpp_args, include_directories: incs, dependencies: deps,
)
prism_test_support_main = static_library(
  'prism_test_support_main', test_main_srcs,
  cpp_args: cpp_args, include_directories: incs, dependencies: deps,
)
test_deps = deps + [declare_dependency(
  link_with: [prism_test_support, prism_test_support_main],
)]

subdir('Algorithm')
subdir('Core')
subdir('Containers')
subdir('Debug')
subdir('Math')
subdir('Memory')
subdir('Ranges')
subdir('String')
subdir('Test')
subdir('Utility')

