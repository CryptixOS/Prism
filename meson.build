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
  'default_library=static',
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
  '-Wno-unused-parameter',
]
macros = [
  '-DFMT_STATIC_THOUSANDS_SEPARATOR=1',
  '-DFMT_USE_LONG_DOUBLE=0',
  '-DFMT_USE_DOUBLE=0',
  '-DFMT_USE_FLOAT=0',
  '-DFMT_USE_FLOAT128=0',
  '-DFMT_USE_EXCEPTIONS=0',
  '-DFMT_HEADER_ONLY=1',
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
  '-MP',
]

incs = [ include_directories('Source') ]
extraincs = get_option('extra_incs')

if extraincs != ''
  cxx_args += '-I' + extraincs
  message('\n\n\n\nextra incs: ', extraincs)
else 
  message('no extra incs\n\n\n\n')
endif

# ==============================================================================
### Dependencies
deps = [
  subproject('fmt-10.2.0').get_variable('fmt_dep'),
  dependency('demangler'),
  dependency('magic_enum', default_options: [
    'test=false',
  ]),
]

subdir('Source')
subdir('Tests')

pkg = import('pkgconfig')
prism = library('prism',
  srcs, cpp_args: cxx_args,
  include_directories: incs,
  dependencies: deps,
  install: true,
)

prism_dep = declare_dependency(
  include_directories : incs,
  compile_args: cxx_args,
  dependencies: deps, 
)
pkg.generate(prism)
install_headers(
  #'Source/Prism/Algorithm/Random.hpp',
  'Source/Prism/Algorithm/SearchString.hpp',

  subdir: 'Source/Prism/Algorithm'
)
install_headers(
  'Source/Prism/Containers/Array.hpp',
  'Source/Prism/Containers/Bitmap.hpp',
  'Source/Prism/Containers/CircularQueue.hpp',
  'Source/Prism/Containers/Deque.hpp',
  'Source/Prism/Containers/DoublyLinkedList.hpp',
  'Source/Prism/Containers/IntrusiveList.hpp',
  'Source/Prism/Containers/IntrusiveList.inl',
  'Source/Prism/Containers/RedBlackTree.hpp',
  'Source/Prism/Containers/RedBlackTree.inl',
  'Source/Prism/Containers/RingBuffer.hpp',
  'Source/Prism/Containers/Span.hpp',
  'Source/Prism/Containers/SplitBuffer.hpp',
  'Source/Prism/Containers/Tuple.hpp',
  'Source/Prism/Containers/UnorderedMap.hpp',
  'Source/Prism/Containers/UnorderedMap.inl',
  'Source/Prism/Containers/Vector.hpp',

  subdir: 'Prism/Containers'
)
install_headers(
  'Source/Prism/Core/Compiler.hpp',
  'Source/Prism/Core/Concepts.hpp',
  'Source/Prism/Core/Config.hpp',
  'Source/Prism/Core/Core.hpp',
  'Source/Prism/Core/Iterator.hpp',
  'Source/Prism/Core/Limits.hpp',
  'Source/Prism/Core/NonCopyable.hpp',
  'Source/Prism/Core/NonMovable.hpp',
  'Source/Prism/Core/Platform.hpp',
  'Source/Prism/Core/Singleton.hpp',
  'Source/Prism/Core/Types.hpp',
  'Source/Prism/Core/TypeTraits.hpp',
  'Source/Prism/Core/TypeTraitsForward.hpp',
  'Source/Prism/Core/UUID.hpp',

  subdir: 'Prism/Core'
)
install_headers(
  'Source/Prism/Core/Details/CallableTraits.inl',
  'Source/Prism/Core/Details/Conditional.inl',
  'Source/Prism/Core/Details/Constructibility.inl',
  'Source/Prism/Core/Details/CoreTraits.inl',
  'Source/Prism/Core/Details/CVRefTraits.inl',
  'Source/Prism/Core/Details/TransformationTraits.inl',
  'Source/Prism/Core/Details/TypeProperties.inl',

  subdir: 'Prism/Core/Details'
)

install_headers(
  'Source/Prism/Debug/Assertions.hpp',
  'Source/Prism/Debug/Log.hpp',
  'Source/Prism/Debug/LogSink.hpp',
  'Source/Prism/Debug/SourceLocation.hpp',
  'Source/Prism/Debug/Stacktrace.hpp',
  subdir: 'Prism/Debug'
)
install_headers(
  'Source/Prism/Memory/BitStream.hpp',
  'Source/Prism/Memory/Buffer.hpp',
  'Source/Prism/Memory/ByteStream.hpp',
  'Source/Prism/Memory/Endian.hpp',
  'Source/Prism/Memory/Pointer.hpp',
  'Source/Prism/Memory/Ref.hpp',
  'Source/Prism/Memory/Scope.hpp',
  'Source/Prism/Memory/WeakRef.hpp',
  subdir: 'Prism/Memory'
)
install_headers(
  'Source/Prism/String/CharTraits.hpp',
  #'Source/Prism/String/FormatString.hpp',
  'Source/Prism/String/Formatter.hpp',
  'Source/Prism/String/Printf.hpp',
  'Source/Prism/String/String.hpp',
  'Source/Prism/String/StringBuilder.hpp',
  'Source/Prism/String/StringUtils.hpp',
  'Source/Prism/String/StringView.hpp',
  'Source/Prism/String/StringView.inl',
  subdir: 'Prism/String'
)
install_headers(
  'Source/Prism/Utility/Atomic.hpp',
  'Source/Prism/Utility/AtomicBuiltins.hpp',
  'Source/Prism/Utility/Checksum.hpp',
  'Source/Prism/Utility/Delegate.hpp',
  #'Source/Prism/Utility/Expected.hpp',
  'Source/Prism/Utility/Hash.hpp',
  'Source/Prism/Utility/Math.hpp',
  'Source/Prism/Utility/Optional.hpp',
  #'Source/Prism/Utility/OptionParser.hpp',
  'Source/Prism/Utility/Path.hpp',
  'Source/Prism/Utility/PathView.hpp',
  'Source/Prism/Utility/Time.hpp',
  subdir: 'Prism/Utility'
)

dep = declare_dependency(
  compile_args: cxx_args,
  include_directories: incs,
  dependencies: prism_dep,
  link_with: prism
)
