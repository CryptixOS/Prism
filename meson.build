#*
#* Created by v1tr10l7 on 20.02.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/
project('prism', ['c', 'cpp'],  license: 'GPL-3', default_options: [
  'c_std=c17',
  'cpp_std=c++23',
  'warning_level=2',
  'werror=true',
],)

if not meson.get_compiler('cpp').has_argument('-std=gnu++23')
  error('C++23 support is needed. Please install clang 17 or higher.')
endif

debug_args = [
  '-fno-lto',
  '-fsized-deallocation',
  #'-fsanitize=undefined',
  #'-fsanitize=address',
]
warning_args = [
  '-Wno-unused-parameter',
  '-Wno-frame-address',
]
macros = [
  '-DFMT_STATIC_THOUSANDS_SEPARATOR=1',
  '-DFMT_USE_LONG_DOUBLE=0',
  '-DFMT_USE_DOUBLE=0',
  '-DFMT_USE_FLOAT=0',
  '-DFMT_USE_FLOAT128=0',
  '-DMAGIC_ENUM_RANGE_MAX=0x8a00',
]

cxx_args = [
  debug_args,
  warning_args,
  macros,
  '-fno-omit-frame-pointer',
  '-fPIE',
  '-fPIC',
  '-static',
  '-fno-rtti',
  '-fno-exceptions',
  '-Wno-frame-address',
  '-MP'
]

incs = [ include_directories('Source') ]

subdir('Source')
#subdir('Tests')

# ==============================================================================
### Dependencies
deps = [
  dependency('demangler'),
  dependency('magic_enum', default_options: [
    'test=false',
  ]),
]

prism = library('prism',
  srcs, cpp_args: cxx_args,
  include_directories: incs,
  dependencies: deps,
)
prism_dep = declare_dependency(
  link_with: prism,
  include_directories : incs,
  compile_args: cxx_args,
  dependencies: deps,
)
