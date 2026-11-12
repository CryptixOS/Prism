#*
#* Created by v1tr10l7 on 20.02.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-2.0-or-later
#*/
# DEPRECATED: CMake (CMakeLists.txt) is Prism's build system now. This file
# is only kept for backward compatibility and will be removed eventually.
project('prism', ['c', 'cpp'],
  version: '0.0.1', license: 'GPL-2.0-or-later',
  default_options: [
    'c_std=c17',
    'cpp_std=c++23',
    'warning_level=2',
    'werror=true',
    'default_library=static',
],)

warning('Building Prism with meson is deprecated, use CMake instead (see README.md)')

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
target = get_option('target')

# The patched fmt 10.2 (fmt-10.2.0.patch) is only needed on CryptixOS; host
# builds (target=app) use the host's fmt.
use_system_fmt = target == 'app'
fmt_macros = []
if not use_system_fmt
  fmt_macros = [
    '-DFMT_STATIC_THOUSANDS_SEPARATOR=1',
    '-DFMT_USE_LONG_DOUBLE=0',
    '-DFMT_USE_DOUBLE=0',
    '-DFMT_USE_FLOAT=0',
    '-DFMT_USE_FLOAT128=0',
    '-DFMT_USE_EXCEPTIONS=0',
    '-DFMT_HEADER_ONLY=1',
  ]
endif
macros = [
  fmt_macros,
  '-DMAGIC_ENUM_RANGE_MAX=0x8a00',
  '-DPRISM_LOG_ENABLE=true',
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
  '-lgcc_s',
]

incs = [ include_directories('Source') ]
extraincs = get_option('extra_incs')
build_tests = get_option('build_tests')

if target == 'cryptix' or target == 'carbonc'
  macros += '-DPRISM_USE_NAMESPACE'

  if target == 'cryptix'
    macros += '-DPRISM_TARGET_CRYPTIX'
  else 
    macros += '-DPRISM_TARGET_CARBONC'
  endif 
endif

if extraincs != ''
  cxx_args += '-I' + extraincs
  message('\n\n\n\nextra incs: ', extraincs)
else 
  message('no extra incs\n\n\n\n')
endif
git_tag = run_command('git', 'rev-parse', 'HEAD').stdout().strip()

version = meson.project_version().split('.')
versionConf = configuration_data()
versionConf.set('library_name', 'prism')
versionConf.set('PM_VERSION_MAJOR', version[0])
versionConf.set('PM_VERSION_MINOR', version[1])
versionConf.set('PM_VERSION_PATCH', version[2])
versionConf.set('git_tag', git_tag)
versionConf.set('arch', host_machine.cpu_family())
cxx = meson.get_compiler('cpp')
versionConf.set('compiler', cxx.get_id())
versionConf.set('compiler_version', cxx.version())

library_version_string_template = '@0@.@1@.@2@-@3@'
versionConf.set('library_version_string', library_version_string_template.format(version[0], version[1], version[2], git_tag))

configure_file(
  input: 'Source/Prism/Core/Version.hpp.in', 
  output: 'Version.hpp',
  configuration: versionConf)

# ==============================================================================
### Dependencies
if use_system_fmt
  fmt_dep = dependency('fmt')
else
  fmt_dep = subproject('fmt-10.2.0').get_variable('fmt_dep')
endif
deps = [
  fmt_dep,
  dependency('demangler'),
  dependency('magic_enum', default_options: [
    'test=false',
  ]),
]

subdir('Source')
if build_tests
  subdir('Tests')
endif

pkg = import('pkgconfig')
prism = static_library('prism',
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
pkg.generate(prism,
  description: 'Freestanding C++23 standard library and runtime for kernel-space development',
  url: 'https://github.com/CryptixOS/Prism',
)

# Unit testing framework (Prism/Test), only for hosted targets.
if target == 'app' or target == 'cryptix-app'
  prism_test = static_library('prism_test',
    test_framework_srcs, cpp_args: cxx_args,
    include_directories: incs,
    dependencies: deps,
    link_with: prism,
    install: true,
  )
  prism_test_main = static_library('prism_test_main',
    test_main_srcs, cpp_args: cxx_args,
    include_directories: incs,
    dependencies: deps,
    install: true,
  )
  prism_test_dep = declare_dependency(
    include_directories: incs,
    compile_args: cxx_args,
    dependencies: deps,
    link_with: [prism_test, prism_test_main, prism],
  )
endif
# Prism is made of C++20 named modules (.ixx), apart from the few headers that
# hold macros. meson can't build named modules, so this build only installs the
# sources; build Prism with CMake.
install_headers(
  #'Source/Prism/Algorithm/Random.hpp',
  'Source/Prism/Algorithm/Find.ixx',
  'Source/Prism/Algorithm/FNV1aHash.ixx',
  'Source/Prism/Algorithm/Hash.ixx',
  'Source/Prism/Algorithm/SearchString.ixx',
  'Source/Prism/Algorithm/Sort.ixx',

  subdir: 'Prism/Algorithm'
)
install_headers(
  # 'Source/Prism/Containers/Any.hpp',
  'Source/Prism/Containers/Array.ixx',
  'Source/Prism/Containers/BitSpan.ixx',
  'Source/Prism/Containers/Bitmap.ixx',
  'Source/Prism/Containers/CircularQueue.ixx',
  'Source/Prism/Containers/Deque.ixx',
  'Source/Prism/Containers/DoublyLinkedList.ixx',
  'Source/Prism/Containers/IntervalTree.ixx',
  'Source/Prism/Containers/IntrusiveList.ixx',
  'Source/Prism/Containers/IntrusiveListV3.ixx',
  'Source/Prism/Containers/IntrusiveRedBlackTree.ixx',
  'Source/Prism/Containers/IntrusiveRefList.ixx',
  'Source/Prism/Containers/KeyValuePair.ixx',
  'Source/Prism/Containers/Queue.ixx',
  'Source/Prism/Containers/RadixTree.ixx',
  'Source/Prism/Containers/RedBlackTree.ixx',
  'Source/Prism/Containers/RingBuffer.ixx',
  'Source/Prism/Containers/Set.ixx',
  'Source/Prism/Containers/Span.ixx',
  'Source/Prism/Containers/SplitBuffer.ixx',
  'Source/Prism/Containers/Stack.ixx',
  'Source/Prism/Containers/Tuple.ixx',
  'Source/Prism/Containers/UnorderedMap.ixx',
  'Source/Prism/Containers/Vector.ixx',

  subdir: 'Prism/Containers'
)
install_headers(
  'Source/Prism/Core/AllocatorTraits.ixx',
  'Source/Prism/Core/Bits.ixx',
  'Source/Prism/Core/Compiler.hpp',
  'Source/Prism/Core/Concepts.ixx',
  'Source/Prism/Core/Config.hpp',
  'Source/Prism/Core/Core.ixx',
  'Source/Prism/Core/Error.hpp',
  'Source/Prism/Core/Error.ixx',
  'Source/Prism/Core/Eternal.ixx',
  'Source/Prism/Core/HashTraits.ixx',
  'Source/Prism/Core/Integer128.ixx',
  'Source/Prism/Core/Iterator.ixx',
  'Source/Prism/Core/Limits.ixx',
  'Source/Prism/Core/NonCopyable.ixx',
  'Source/Prism/Core/NonMovable.ixx',
  'Source/Prism/Core/ParameterPack.ixx',
  'Source/Prism/Core/Platform.hpp',
  'Source/Prism/Core/Ranges.ixx',
  'Source/Prism/Core/Singleton.ixx',
  'Source/Prism/Core/Types.hpp',
  'Source/Prism/Core/TypeTraits.ixx',
  'Source/Prism/Core/TypeTraitsForward.ixx',
  'Source/Prism/Core/UUID.ixx',
  # meson.build_root() / 'Version.hpp',

  subdir: 'Prism/Core'
)
install_headers(
  'Source/Prism/Core/Details/SignModifiers.inl',

  subdir: 'Prism/Core/Details'
)

install_headers(
  'Source/Prism/Debug/Assertions.hpp',
  'Source/Prism/Debug/Assertions.ixx',
  'Source/Prism/Debug/Log.hpp',
  'Source/Prism/Debug/Log.ixx',
  'Source/Prism/Debug/Logger.ixx',
  'Source/Prism/Debug/LogSink.ixx',
  'Source/Prism/Debug/SourceLocation.ixx',
  'Source/Prism/Debug/Stacktrace.ixx',
  subdir: 'Prism/Debug'
)
install_headers(
  'Source/Prism/Memory/Allocator.ixx',
  'Source/Prism/Memory/BitStream.ixx',
  'Source/Prism/Memory/Buffer.ixx',
  'Source/Prism/Memory/ByteStream.ixx',
  'Source/Prism/Memory/Endian.ixx',
  'Source/Prism/Memory/Memory.ixx',
  'Source/Prism/Memory/Pointer.ixx',
  'Source/Prism/Memory/Ref.ixx',
  'Source/Prism/Memory/RefCounted.ixx',
  'Source/Prism/Memory/Scope.ixx',
  'Source/Prism/Memory/SlabAllocator.ixx',
  'Source/Prism/Memory/SlabPool.ixx',
  'Source/Prism/Memory/WeakRef.ixx',
  subdir: 'Prism/Memory'
)

install_headers(
  'Source/Prism/Network/IPv4Address.ixx',
  subdir: 'Prism/Network'
)

install_headers(
  'Source/Prism/Ranges/Range.ixx',
  'Source/Prism/Ranges/Ranges.ixx',
  'Source/Prism/Ranges/Views.ixx',
  subdir: 'Prism/Ranges'
)
install_headers(
  'Source/Prism/String/CharTraits.ixx',
  'Source/Prism/String/CodePoints.ixx',
  'Source/Prism/String/FormatHandler.ixx',
  'Source/Prism/String/FormatString.ixx',
  'Source/Prism/String/Formatter.ixx',
  'Source/Prism/String/FormatterContext.ixx',
  'Source/Prism/String/Printf.ixx',
  'Source/Prism/String/String.ixx',
  'Source/Prism/String/StringBuilder.ixx',
  'Source/Prism/String/StringUtils.ixx',
  'Source/Prism/String/StringView.ixx',
  'Source/Prism/String/StringView.ixx',
  subdir: 'Prism/String'
)
install_headers(
  'Source/Prism/Test/CrashTest.ixx',
  'Source/Prism/Test/Macros.hpp',
  'Source/Prism/Test/Macros.ixx',
  'Source/Prism/Test/Test.hpp',
  'Source/Prism/Test/Test.ixx',
  'Source/Prism/Test/TestCase.hpp',
  'Source/Prism/Test/TestCase.ixx',
  'Source/Prism/Test/TestResult.ixx',
  'Source/Prism/Test/TestSuite.ixx',
  subdir: 'Prism/Test'
)
install_headers(
  'Source/Prism/Test/Randomized/Generator.ixx',
  'Source/Prism/Test/Randomized/RandomRun.ixx',
  'Source/Prism/Test/Randomized/RandSource.ixx',
  subdir: 'Prism/Test/Randomized'
)
install_headers(
  'Source/Prism/Utility/Atomic.ixx',
  'Source/Prism/Utility/AtomicBuiltins.ixx',
  'Source/Prism/Utility/Checksum.ixx',
  'Source/Prism/Utility/Compare.ixx',
  'Source/Prism/Utility/Delegate.ixx',
  'Source/Prism/Utility/Enum.ixx',
  'Source/Prism/Utility/Expected.ixx',
  'Source/Prism/Utility/Hash.ixx',
  'Source/Prism/Utility/LockingPolicy.ixx',
  'Source/Prism/Utility/Math.ixx',
  'Source/Prism/Utility/Mutex.ixx',
  'Source/Prism/Utility/Optional.ixx',
  'Source/Prism/Utility/OptionParser.ixx',
  'Source/Prism/Utility/Path.ixx',
  'Source/Prism/Utility/PathView.ixx',
  'Source/Prism/Utility/RangeAllocator.ixx',
  'Source/Prism/Utility/SimdIntrinsics.ixx',
  'Source/Prism/Utility/SimdVector.ixx',
  'Source/Prism/Utility/Time.ixx',
  'Source/Prism/Utility/Variant.ixx',
  subdir: 'Prism/Utility'
)

dep = declare_dependency(
  compile_args: cxx_args,
  include_directories: incs,
  dependencies: prism_dep,
  link_with: prism,
)
