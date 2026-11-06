/*
 * Created by v1tr10l7 on 06.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>

/**
 * MakeSigned / MakeUnsigned (std::make_signed / std::make_unsigned).
 *
 * This file is included by Types.hpp, before any other trait exists, so it
 * only relies on compiler builtins and its own helpers.
 *
 *  - Standard integer types map to their counterpart of the same rank.
 *  - Enumerations and the character types (char8_t, char16_t, char32_t,
 *    wchar_t) map to the integer type of the smallest rank that has the
 *    same size.
 *  - cv-qualifiers are preserved.
 *  - bool and non-integral types have no `Type` member (SFINAE friendly).
 */
namespace Prism
{
    namespace Details
    {
        template <__SIZE_TYPE__ Size>
        struct IntegerOfSize
        {
        };
        template <>
        struct IntegerOfSize<1>
        {
            using Signed   = signed char;
            using Unsigned = unsigned char;
        };
        template <>
        struct IntegerOfSize<sizeof(short)>
        {
            using Signed   = short;
            using Unsigned = unsigned short;
        };
        template <>
        struct IntegerOfSize<sizeof(int)>
        {
            using Signed   = int;
            using Unsigned = unsigned int;
        };
#if __SIZEOF_LONG__ != __SIZEOF_INT__
        template <>
        struct IntegerOfSize<sizeof(long)>
        {
            using Signed   = long;
            using Unsigned = unsigned long;
        };
#endif
#if __SIZEOF_LONG_LONG__ != __SIZEOF_LONG__
        template <>
        struct IntegerOfSize<sizeof(long long)>
        {
            using Signed   = long long;
            using Unsigned = unsigned long long;
        };
#endif
#ifdef __SIZEOF_INT128__
        template <>
        struct IntegerOfSize<16>
        {
            using Signed   = __int128;
            using Unsigned = unsigned __int128;
        };
#endif

        template <typename T, typename S, typename U>
        struct SignPair
        {
            using Signed   = S;
            using Unsigned = U;
        };

        // Primary: no members, so MakeSigned<float> etc. are SFINAE friendly.
        template <typename T, bool = __is_enum(T)>
        struct SignInfo
        {
        };
        // Enumerations: integer of the same size.
        template <typename T>
        struct SignInfo<T, true> : IntegerOfSize<sizeof(T)>
        {
        };

#define PrismDefineSignInfo(type, s, u)                                        \
    template <>                                                                \
    struct SignInfo<type, false> : SignPair<type, s, u>                        \
    {                                                                          \
    }

        PrismDefineSignInfo(char, signed char, unsigned char);
        PrismDefineSignInfo(signed char, signed char, unsigned char);
        PrismDefineSignInfo(unsigned char, signed char, unsigned char);
        PrismDefineSignInfo(short, short, unsigned short);
        PrismDefineSignInfo(unsigned short, short, unsigned short);
        PrismDefineSignInfo(int, int, unsigned int);
        PrismDefineSignInfo(unsigned int, int, unsigned int);
        PrismDefineSignInfo(long, long, unsigned long);
        PrismDefineSignInfo(unsigned long, long, unsigned long);
        PrismDefineSignInfo(long long, long long, unsigned long long);
        PrismDefineSignInfo(unsigned long long, long long, unsigned long long);
#ifdef __SIZEOF_INT128__
        PrismDefineSignInfo(__int128, __int128, unsigned __int128);
        PrismDefineSignInfo(unsigned __int128, __int128, unsigned __int128);
#endif
#undef PrismDefineSignInfo

        // Character types: integer of the same size.
        template <>
        struct SignInfo<wchar_t, false> : IntegerOfSize<sizeof(wchar_t)>
        {
        };
        template <>
        struct SignInfo<char8_t, false> : IntegerOfSize<sizeof(char8_t)>
        {
        };
        template <>
        struct SignInfo<char16_t, false> : IntegerOfSize<sizeof(char16_t)>
        {
        };
        template <>
        struct SignInfo<char32_t, false> : IntegerOfSize<sizeof(char32_t)>
        {
        };

        // cv handling, without depending on RemoveCv (not defined yet).
        template <typename T>
        struct SignCv
        {
            using Base = T;
            template <typename U>
            using Apply = U;
        };
        template <typename T>
        struct SignCv<const T>
        {
            using Base = T;
            template <typename U>
            using Apply = const U;
        };
        template <typename T>
        struct SignCv<volatile T>
        {
            using Base = T;
            template <typename U>
            using Apply = volatile U;
        };
        template <typename T>
        struct SignCv<const volatile T>
        {
            using Base = T;
            template <typename U>
            using Apply = const volatile U;
        };

        template <typename T>
        struct MakeSignedImpl
        {
        };
        template <typename T>
            requires requires {
                typename SignInfo<typename SignCv<T>::Base>::Signed;
            }
        struct MakeSignedImpl<T>
        {
            using Type = typename SignCv<T>::template Apply<
                typename SignInfo<typename SignCv<T>::Base>::Signed>;
        };

        template <typename T>
        struct MakeUnsignedImpl
        {
        };
        template <typename T>
            requires requires {
                typename SignInfo<typename SignCv<T>::Base>::Unsigned;
            }
        struct MakeUnsignedImpl<T>
        {
            using Type = typename SignCv<T>::template Apply<
                typename SignInfo<typename SignCv<T>::Base>::Unsigned>;
        };
    } // namespace Details

    /// Signed integer type corresponding to T, cv-qualifiers preserved.
    template <typename T>
    struct MakeSigned : Details::MakeSignedImpl<T>
    {
    };
    /// Unsigned integer type corresponding to T, cv-qualifiers preserved.
    template <typename T>
    struct MakeUnsigned : Details::MakeUnsignedImpl<T>
    {
    };

    template <typename T>
    using MakeSignedType = typename MakeSigned<T>::Type;
    template <typename T>
    using MakeUnsignedType = typename MakeUnsigned<T>::Type;
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::MakeSigned;
using Prism::MakeSignedType;
using Prism::MakeUnsigned;
using Prism::MakeUnsignedType;
#endif
