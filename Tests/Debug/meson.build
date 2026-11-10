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

# Same test with Logger taking Prism::FormatString instead of fmt's.
test_log_prism_format = executable(
  'LogPrismFormat', files('Log' / 'main.cpp'),
  cpp_args: cpp_args + ['-DPRISM_LOGGER_USE_PRISM_FORMAT=1'],
  link_args: link_args,
  include_directories: incs, dependencies: test_deps
)
test('LogPrismFormat', test_log_prism_format, suite: 'Debug')
