/*
 * Created by v1tr10l7 on 16.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>

#if PrismHasInclude(<assert.h>)
    #include <assert.h>
#else
extern "C" PM_NORETURN void __assert_fail(const char* expr, const char* file,
                                          unsigned int line,
                                          const char*  function);
    #define assert(expr)                                                       \
        (__builtin_expect(!(expr), 0)                                          \
             ? __assert_fail(#expr, __FILE__, __LINE__, __PRETTY_FUNCTION__)   \
             : (void)0)
#endif
#if PRISM_DISABLE_FMT == 0
    #include <fmt/format.h>
#endif

#define PrismStringifyInner(x) #x
#define PrismStringify(x)      PrismStringifyInner(x)

#define Stringify(x)           PrismStringify(x)

namespace Prism
{
    template <typename T>
    struct MakeSigned
    {
        using Type = void;
    };
    template <>
    struct MakeSigned<signed char>
    {
        using Type = signed char;
    };
    template <>
    struct MakeSigned<short>
    {
        using Type = short;
    };
    template <>
    struct MakeSigned<int>
    {
        using Type = int;
    };
    template <>
    struct MakeSigned<long>
    {
        using Type = long;
    };
    template <>
    struct MakeSigned<long long>
    {
        using Type = long long;
    };
    template <>
    struct MakeSigned<unsigned char>
    {
        using Type = char;
    };
    template <>
    struct MakeSigned<unsigned short>
    {
        using Type = short;
    };
    template <>
    struct MakeSigned<unsigned int>
    {
        using Type = int;
    };
    template <>
    struct MakeSigned<unsigned long>
    {
        using Type = long;
    };
    template <>
    struct MakeSigned<unsigned long long>
    {
        using Type = long long;
    };
    template <>
    struct MakeSigned<char>
    {
        using Type = char;
    };
    template <typename T>
    using MakeSignedType = typename MakeSigned<T>::Type;

    using PhysAddr       = __UINTPTR_TYPE__;

    using usize          = __SIZE_TYPE__;
    using isize          = MakeSignedType<usize>;

#if PRISM_TARGET_CRYPTIX == 0
    using f32 = float;
    using f64 = double;
#endif

    using u8       = __UINT8_TYPE__;
    using u16      = __UINT16_TYPE__;
    using u32      = __UINT32_TYPE__;
    using u64      = __UINT64_TYPE__;

    using i8       = __INT8_TYPE__;
    using i16      = __INT16_TYPE__;
    using i32      = __INT32_TYPE__;
    using i64      = __INT64_TYPE__;

    using pointer  = __UINTPTR_TYPE__;
    using upointer = __UINTPTR_TYPE__;
    using ipointer = __INTPTR_TYPE__;

    using ptrdiff  = __PTRDIFF_TYPE__;
    using NullType = decltype(nullptr);

    using symbol   = void*[];
    constexpr u64 Bit(u64 n) { return (1ull << n); }

    template <typename T>
    constexpr const T& Min(const T& lhs, const T& rhs)
    {
        return lhs < rhs ? lhs : rhs;
    }

    using WeakOrdering   = std::weak_ordering;
    using StrongOrdering = std::strong_ordering;
    template <typename T>
    using InitializerList = std::initializer_list<T>;

    template <typename T>
    constexpr const T& Min(InitializerList<T> ilist)
    {
        auto     it     = ilist.begin();
        const T& result = *it++;
        for (; it != ilist.end(); it++)
            if (*it < result) result = *it;

        return result;
    }
    template <typename T>
    constexpr const T& Max(const T& lhs, const T& rhs)
    {
        return lhs > rhs ? lhs : rhs;
    }
    template <typename T>
    constexpr const T& Max(InitializerList<T> ilist)
    {
        auto     it     = ilist.begin();
        const T& result = *it++;
        for (; it != ilist.end(); it++)
            if (*it > result) result = *it;

        return result;
    }

    template <typename T>
    struct Hash;

    enum class AlignmentType : usize
    {
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::PhysAddr;

using Prism::isize;
using Prism::usize;

using Prism::u16;
using Prism::u32;
using Prism::u64;
using Prism::u8;

using Prism::i16;
using Prism::i32;
using Prism::i64;
using Prism::i8;

using Prism::AlignmentType;
using Prism::ipointer;
using Prism::pointer;
using Prism::ptrdiff;
using Prism::upointer;

using Prism::symbol;

using Prism::Bit;
using Prism::InitializerList;
using Prism::Max;
using Prism::Min;
using Prism::StrongOrdering;
using Prism::WeakOrdering;
#endif
