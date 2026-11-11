/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * The Prism module: `import Prism;`
 *
 * One module interface unit built from the regular headers, the same way
 * fmt builds its module. Every Prism header is included in the module
 * purview, where PRISM_EXPORT expands to `export` (PRISM_MODULE_BUILD is
 * defined for this TU only). The header build of the same files stays
 * untouched, so `#include <Prism/...>` keeps working.
 *
 * Macros can't be exported. Importers that need them also do
 * `#include <Prism/Macros.hpp>`.
 *
 * Rules for the headers:
 *  - Anything that isn't Prism (std, libc, fmt, magic_enum, demangler,
 *    platform headers) has to be included in the global module fragment
 *    below first. Include guards then turn the headers' own includes into
 *    no-ops, so third-party declarations never get attached to Prism.
 *  - Exported namespaces can't contain internal-linkage entities: use
 *    `inline constexpr` for namespace-scope constants, and keep `static`
 *    helpers in .cpp files or in a non-exported namespace.
 */
module;

// ---------------------------------------------------- global module fragment
#include <Prism/Macros.hpp>

#include <compare>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <ranges>
#include <utility>

#include <math.h>
#if PrismHasInclude(<sched.h>)
    #include <sched.h>
#endif
#if PrismHasInclude(<unistd.h>)
    #include <unistd.h>
#endif
#if PRISM_TARGET_CRYPTIX != 0
    #include <API/Limits.hpp>
    #include <Library/Stacktrace.hpp>
    #include <cerrno>
#elif PRISM_TARGET_CARBONC != 0
    #include <errno.h>
#endif
#if defined(__APPLE__)
    #include <TargetConditionals.h>
#endif
#if defined(__x86_64__) || defined(__i386__)
    #include <emmintrin.h>
#elif defined(__aarch64__)
    #include <arm_neon.h>
#endif

#include <demangler.hpp>
#include <magic_enum/magic_enum.hpp>

export module Prism;

// -------------------------------------------------------------- interface
// clang-format off
#include <Prism/Core/AllocatorTraits.hpp>
#include <Prism/Core/Bits.hpp>
#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Core.hpp>
#include <Prism/Core/Error.hpp>
#include <Prism/Core/Eternal.hpp>
#include <Prism/Core/HashTraits.hpp>
#include <Prism/Core/Integer128.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/Limits.hpp>
#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/NonMovable.hpp>
#include <Prism/Core/ParameterPack.hpp>
#include <Prism/Core/Ranges.hpp>
#include <Prism/Ranges/Range.hpp>
#include <Prism/Ranges/Ranges.hpp>
#include <Prism/Ranges/Views.hpp>
#include <Prism/Core/Singleton.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Core/UUID.hpp>

#include <Prism/Debug/Assertions.hpp>
#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/LogSink.hpp>
#include <Prism/Debug/Logger.hpp>
#include <Prism/Debug/SourceLocation.hpp>
#include <Prism/Debug/Stacktrace.hpp>

#include <Prism/Algorithm/FNV1aHash.hpp>
#include <Prism/Algorithm/Find.hpp>
#include <Prism/Algorithm/Hash.hpp>
#include <Prism/Algorithm/Random.hpp>
#include <Prism/Algorithm/SearchString.hpp>
#include <Prism/Algorithm/Sort.hpp>

#include <Prism/Containers/Array.hpp>
#include <Prism/Containers/BitSpan.hpp>
#include <Prism/Containers/Bitmap.hpp>
#include <Prism/Containers/CircularQueue.hpp>
#include <Prism/Containers/Deque.hpp>
#include <Prism/Containers/DoublyLinkedList.hpp>
#include <Prism/Containers/IntervalTree.hpp>
#include <Prism/Containers/IntrusiveList.hpp>
#include <Prism/Containers/IntrusiveListV3.hpp>
#include <Prism/Containers/IntrusiveRedBlackTree.hpp>
#include <Prism/Containers/IntrusiveRefList.hpp>
#include <Prism/Containers/KeyValuePair.hpp>
#include <Prism/Containers/Queue.hpp>
#include <Prism/Containers/RadixTree.hpp>
#include <Prism/Containers/RedBlackTree.hpp>
#include <Prism/Containers/RingBuffer.hpp>
#include <Prism/Containers/Set.hpp>
#include <Prism/Containers/Span.hpp>
#include <Prism/Containers/SplitBuffer.hpp>
#include <Prism/Containers/Stack.hpp>
#include <Prism/Containers/Tuple.hpp>
#include <Prism/Containers/UnorderedMap.hpp>
#include <Prism/Containers/Vector.hpp>

#include <Prism/Math/Matrix.hpp>
#include <Prism/Math/SimdVector.hpp>
#include <Prism/Math/Vector.hpp>

#include <Prism/Memory/Allocator.hpp>
#include <Prism/Memory/BitStream.hpp>
#include <Prism/Memory/Buffer.hpp>
#include <Prism/Memory/BumpAllocator.hpp>
#include <Prism/Memory/ByteStream.hpp>
#include <Prism/Memory/Endian.hpp>
#include <Prism/Memory/Memory.hpp>
#include <Prism/Memory/Pointer.hpp>
#include <Prism/Memory/Ref.hpp>
#include <Prism/Memory/RefCounted.hpp>
#include <Prism/Memory/SSOStorage.hpp>
#include <Prism/Memory/Scope.hpp>
#include <Prism/Memory/SlabAllocator.hpp>
#include <Prism/Memory/SlabPool.hpp>
#include <Prism/Memory/SlobAllocator.hpp>
#include <Prism/Memory/WeakRef.hpp>

#include <Prism/Network/IPv4Address.hpp>

#include <Prism/String/CharTraits.hpp>
#include <Prism/String/CodePoints.hpp>
#include <Prism/String/FormatHandler.hpp>
#include <Prism/String/FormatString.hpp>
#include <Prism/String/FormatStyle.hpp>
#include <Prism/String/Formatter.hpp>
#include <Prism/String/FormatterContext.hpp>
#include <Prism/String/Printf.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringBuilder.hpp>
#include <Prism/String/StringUtils.hpp>
#include <Prism/String/StringView.hpp>

#include <Prism/Utility/Atomic.hpp>
#include <Prism/Utility/AtomicBuiltins.hpp>
#include <Prism/Utility/Checksum.hpp>
#include <Prism/Utility/Compare.hpp>
#include <Prism/Utility/Delegate.hpp>
#include <Prism/Utility/Expected.hpp>
#include <Prism/Utility/Hash.hpp>
#include <Prism/Utility/LockingPolicy.hpp>
#include <Prism/Utility/Math.hpp>
#include <Prism/Utility/Mutex.hpp>
#include <Prism/Utility/Optional.hpp>
#include <Prism/Utility/Path.hpp>
#include <Prism/Utility/PathView.hpp>
#include <Prism/Utility/RangeAllocator.hpp>
#include <Prism/Utility/SimdIntrinsics.hpp>
#include <Prism/Utility/SimdVector.hpp>
#include <Prism/Utility/Time.hpp>
// clang-format on

// ---------------------------------------------------------- implementation
// The library's .cpp files (Source/meson.build). Ideally these would sit in
// a private module fragment (`module :private;`) so editing them doesn't
// rebuild importers, but GCC doesn't support that yet.

#include <Prism/Algorithm/Hash.cpp>
#include <Prism/Containers/Bitmap.cpp>
#include <Prism/Containers/RingBuffer.cpp>
#include <Prism/Debug/Assertions.cpp>
#include <Prism/Debug/Log.cpp>
#include <Prism/Debug/LogSink.cpp>
#include <Prism/Debug/Logger.cpp>
#include <Prism/Debug/Stacktrace.cpp>
#include <Prism/Debug/Ubsan.cpp>
#include <Prism/Memory/Memory.cpp>
#include <Prism/String/Printf.cpp>
#include <Prism/Utility/Path.cpp>
#include <Prism/Utility/PathView.cpp>
#include <Prism/Utility/Time.cpp>
