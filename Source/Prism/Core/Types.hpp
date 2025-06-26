/*
 * Created by v1tr10l7 on 16.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Config.hpp>

#include <cassert>
#include <fmt/format.h>

#define PrismStringifyInner(x) #x
#define PrismStringify(x)      PrismStringifyInner(x)

#if PRISM_TARGET_CRYPTIX == 1
    #include <cerrno>
    #include <expected>
    #define PRISM_ERRNO_T      std::errno_t;
    #define Stringify(x)       PrismStringify(x)
    #define PRISM_EXPECTED_T   std::expected
    #define PRISM_UNEXPECTED_T std::unexpected
#else
    // #include <Prism/Utility/Expected.hpp>
    #define PRISM_ERRNO_T      i32
    #define PRISM_EXPECTED_T   Expected
    #define PRISM_UNEXPECTED_T Unexpected
#endif

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

    using u8      = __UINT8_TYPE__;
    using u16     = __UINT16_TYPE__;
    using u32     = __UINT32_TYPE__;
    using u64     = __UINT64_TYPE__;

    using i8      = __INT8_TYPE__;
    using i16     = __INT16_TYPE__;
    using i32     = __INT32_TYPE__;
    using i64     = __INT64_TYPE__;

    using pointer = __UINTPTR_TYPE__;
    using ptrdiff = __PTRDIFF_TYPE__;

    using symbol  = void*[];
#if PRISM_TARGET_CRYPTIX == 1
    using ErrorCode = PRISM_ERRNO_T;
    using Error     = PRISM_UNEXPECTED_T<ErrorCode>;

    template <typename R>
    using ErrorOr = PRISM_EXPECTED_T<R, ErrorCode>;
#endif

    constexpr u64 Bit(u64 n) { return (1ull << n); }
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

using Prism::Error;
using Prism::ErrorCode;
using Prism::symbol;

using Prism::ErrorOr;

using Prism::Bit;
#endif
