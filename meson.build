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


sources = files(
  'Source/Prism/Debug/Log.cpp',
  'Source/Prism/Debug/LogSink.cpp',
  'Source/Prism/Debug/Ubsan.cpp',
  'Source/Prism/Debug/Stacktrace.cpp',
  'Source/Prism/Path.cpp',
  'Source/Prism/PathView.cpp',
)

#subdir('Tests')

# ==============================================================================
### Dependencies
prism_deps = [
  #dependency('demangler'),
  #dependency('magic_enum'),
]

prism_cxx_args = [
  '-fno-omit-frame-pointer',
  '-fno-lto',
  '-fsanitize=undefined',
  #'-fsanitize=address',
  '-fstack-protector-all',
  '-mstack-protector-guard=global',
  '-fPIE',
  '-static',
  '-fno-rtti',
  '-fno-exceptions',
  '-fsized-deallocation',
  '-Wno-unused-parameter',
  '-Wno-frame-address',
  '-D__cplusplus=202302L',
  '-D__cpp_concepts=202207L',
  '-DFMT_STATIC_THOUSANDS_SEPARATOR=1',
  '-DFMT_USE_LONG_DOUBLE=0',
  '-DFMT_USE_DOUBLE=0',
  '-DFMT_USE_FLOAT=0',
  '-DFMT_USE_FLOAT128=0',
  '-DMAGIC_ENUM_RANGE_MAX=0x8a00',
  '-MP']

prism_dep = declare_dependency(
  include_directories : 'Source',
  sources: sources,
 # compile_args: cxx_args
)
