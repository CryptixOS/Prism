/*
 * Created by v1tr10l7 on 16.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Macros.hpp>

#if PRISM_DISABLE_FMT == 0
    #include <fmt/format.h>
#endif

#if PrismHasInclude(<compare>)
    #include <compare>
#endif
#if PrismHasInclude(<initializer_list>)
    #include <initializer_list>
#endif

PRISM_EXPORT namespace Prism
{
    template <typename... T>
    constexpr void IgnoreUnused(const T&...)
    {
    }

    /// Aborts execution immediately (a trap instruction where available).
    PM_NORETURN inline void Trap() PM_NOEXCEPT
    {
#if PrismHasBuiltin(__builtin_trap)
        __builtin_trap();
#else
        for (;;);
#endif
    }

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
    using VaList          = __builtin_va_list;

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

#if PRISM_USE_NAMESPACE != 0
PRISM_EXPORT using Prism::PhysAddr;

PRISM_EXPORT using Prism::isize;
PRISM_EXPORT using Prism::usize;

PRISM_EXPORT using Prism::u16;
PRISM_EXPORT using Prism::u32;
PRISM_EXPORT using Prism::u64;
PRISM_EXPORT using Prism::u8;

PRISM_EXPORT using Prism::i16;
PRISM_EXPORT using Prism::i32;
PRISM_EXPORT using Prism::i64;
PRISM_EXPORT using Prism::i8;

PRISM_EXPORT using Prism::AlignmentType;
PRISM_EXPORT using Prism::ipointer;
PRISM_EXPORT using Prism::pointer;
PRISM_EXPORT using Prism::ptrdiff;
PRISM_EXPORT using Prism::upointer;

PRISM_EXPORT using Prism::symbol;

PRISM_EXPORT using Prism::Bit;
PRISM_EXPORT using Prism::InitializerList;
PRISM_EXPORT using Prism::Max;
PRISM_EXPORT using Prism::Min;
PRISM_EXPORT using Prism::StrongOrdering;
PRISM_EXPORT using Prism::VaList;
PRISM_EXPORT using Prism::WeakOrdering;
    #if PRISM_TARGET_CRYPTIX == 0
PRISM_EXPORT using Prism::f32;
PRISM_EXPORT using Prism::f64;
    #endif

PRISM_EXPORT using Prism::IgnoreUnused;
PRISM_EXPORT using Prism::Trap;
#endif
