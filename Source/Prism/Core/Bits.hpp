/*
 * Created by v1tr10l7 on 14.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    template <typename T>
    struct IntegerTraits
    {
        static constexpr i32 Digits = std::numeric_limits<T>::digits;
        static constexpr T   Max    = std::numeric_limits<T>::max();
    };

    template <typename T, typename U>
    [[nodiscard]] PM_ALWAYS_INLINE constexpr T BitCast(U const& a)
        requires(sizeof(T) == sizeof(U))
             && IsTriviallyCopyableV<T> && IsTriviallyCopyableV<U>
    {
#if (__has_builtin(__builtin_bit_cast))
        return __builtin_bit_cast(T, a);
#else
        static_assert(sizeof(T) == sizeof(U));

        T result;
        __builtin_memcpy(&result, &a, sizeof(T));
        return result;
#endif
    }

    template <Integral T>
    [[nodiscard]]
    constexpr T ByteSwap(T valueue) PM_NOEXCEPT
    {
        if constexpr (sizeof(T) == 1) return valueue;
        if !consteval
        {
            if constexpr (sizeof(T) == 2) return __builtin_bswap16(valueue);
            if constexpr (sizeof(T) == 4) return __builtin_bswap32(valueue);
            if constexpr (sizeof(T) == 8) return __builtin_bswap64(valueue);
            if constexpr (sizeof(T) == 16)
#if PrismHasBuiltin(__builtin_bswap128)
                return __builtin_bswap128(valueue);
#else
                return (__builtin_bswap64(valueue >> 64)
                        | (static_cast<T>(__builtin_bswap64(valueue)) << 64));
#endif
        }

        using U          = typename MakeUnsigned<RemoveCvType<T>>::Type;
        usize difference = CHAR_BIT * (sizeof(T) - 1);
        usize mask1      = static_cast<unsigned char>(~0);
        usize mask2      = mask1 << difference;
        usize value      = valueue;
        for (usize i = 0; i < sizeof(T) / 2; ++i)
        {
            usize b1 = value & mask1;
            usize b2 = value & mask2;
            value = (value ^ b1 ^ b2 ^ (b1 << difference) ^ (b2 >> difference));
            mask1 <<= CHAR_BIT;
            mask2 >>= CHAR_BIT;
            difference -= 2 * CHAR_BIT;
        }
        return static_cast<U>(value);
    }

    template <UnsignedIntegral T>
    constexpr T RotateLeft(T x, i32 s) PM_NOEXCEPT
    {
        constexpr auto Nd = IntegerTraits<T>::Digits;
        if constexpr ((Nd & (Nd - 1)) == 0)
        {
            constexpr unsigned uNd = Nd;
            const unsigned     r   = s;
            return (x << (r % uNd)) | (x >> ((-r) % uNd));
        }
        const i32 r = s % Nd;
        if (r == 0) return x;
        else if (r > 0) return (x << r) | (x >> ((Nd - r) % Nd));
        else return (x >> -r) | (x << ((Nd + r) % Nd));
    }

    template <UnsignedIntegral T>
    constexpr T RotateRight(T x, i32 s) PM_NOEXCEPT
    {
        constexpr auto Nd = IntegerTraits<T>::Digits;
        if constexpr ((Nd & (Nd - 1)) == 0)
        {
            constexpr unsigned uNd = Nd;
            const unsigned     r   = s;
            return (x >> (r % uNd)) | (x << ((-r) % uNd));
        }
        const i32 r = s % Nd;
        if (r == 0) return x;
        else if (r > 0) return (x >> r) | (x << ((Nd - r) % Nd));
        else return (x << -r) | (x >> ((Nd + r) % Nd));
    }

    template <UnsignedIntegral T>
    constexpr i32 CountLeftZero(T x) PM_NOEXCEPT
    {
        constexpr auto Nd = IntegerTraits<T>::Digits;

#if PrismHasBuiltin(builtin_clzg)
        return builtin_clzg(x, Nd);
#else
        if (x == 0) return Nd;

        constexpr auto NdUll = IntegerTraits<unsigned long long>::Digits;
        constexpr auto NdUl  = IntegerTraits<unsigned long>::Digits;
        constexpr auto NdU   = IntegerTraits<unsigned>::Digits;

        if constexpr (Nd <= NdU)
        {
            constexpr i32 diff = NdU - Nd;
            return builtin_clz(x) - diff;
        }
        else if constexpr (Nd <= NdUl)
        {
            constexpr i32 diff = NdUl - Nd;
            return builtin_clzl(x) - diff;
        }
        else if constexpr (Nd <= NdUll)
        {
            constexpr i32 diff = NdUll - Nd;
            return builtin_clzll(x) - diff;
        }
        else // (Nd > NdUll)
        {
            static_assert(Nd <= (2 * NdUll),
                          "Maximum supported integer size is 128-bit");

    #if 0
            unsigned long long high = x >> NdUll;
            if (high != 0)
            {
                constexpr i32 diff = (2 * NdUll) - Nd;
                return builtin_clzll(high) - diff;
            }
            constexpr auto     maxUll = IntegerTraits<unsigned long long>::max;
            unsigned long long low    = x & maxUll;
            return (Nd - NdUll) + builtin_clzll(low);
    #endif
            return {};
        }
#endif
    }
    template <UnsignedIntegral T>
    constexpr i32 CountRightZero(T x) PM_NOEXCEPT
    {
        constexpr auto Nd = IntegerTraits<T>::Digits;

#if PrismHasBuiltin(__builtin_ctzg)
        return __builtin_ctzg(x, Nd);
#else
        if (x == 0) return Nd;

        constexpr auto NdUll = IntegerTraits<unsigned long long>::Digits;
        constexpr auto NdUl  = IntegerTraits<unsigned long>::Digits;
        constexpr auto NdU   = IntegerTraits<unsigned>::Digits;

        if constexpr (Nd <= NdU) return __builtin_ctz(x);
        else if constexpr (Nd <= NdUl) return __builtin_ctzl(x);
        else if constexpr (Nd <= NdUll) return __builtin_ctzll(x);
        else
        {
            static_assert(Nd <= (2 * NdUll),
                          "Maximum supported integer size is 128-bit");

            constexpr auto     MaxUll = IntegerTraits<unsigned long long>::Max;
            unsigned long long low    = x & MaxUll;
            if (low != 0) return __builtin_ctzll(low);
            unsigned long long high = x >> NdUll;
            return __builtin_ctzll(high) + NdUll;
        }
#endif
    }

    template <UnsignedIntegral T>
    constexpr i32 CountLeftOne(T x) PM_NOEXCEPT
    {
        return CountLeftZero<T>(static_cast<T>(~x));
    }
    template <UnsignedIntegral T>
    constexpr i32 CountRightOne(T x) PM_NOEXCEPT
    {
        return CountRightZero(static_cast<T>(~x));
    }

    template <UnsignedIntegral T>
    constexpr i32 PopCount(T x) PM_NOEXCEPT
    {
#if PrismHasBuiltin(__builtin_popcountg)
        return __builtin_popcountg(x);
#else
        constexpr auto Nd    = IntegerTraits<T>::Digits;

        constexpr auto NdUll = IntegerTraits<unsigned long long>::Digits;
        constexpr auto NdUl  = IntegerTraits<unsigned long>::Digits;
        constexpr auto NdU   = IntegerTraits<unsigned>::Digits;

        if constexpr (Nd <= NdU) return __builtin_popcount(x);
        else if constexpr (Nd <= NdUl) return __builtin_popcountl(x);
        else if constexpr (Nd <= NdUll) return __builtin_popcountll(x);
        else
        {
            static_assert(Nd <= (2 * NdUll),
                          "Maximum supported integer size is 128-bit");

            constexpr auto     maxUll = IntegerTraits<unsigned long long>::Max;
            unsigned long long low    = x & maxUll;
            unsigned long long high   = x >> NdUll;
            return __builtin_popcountll(low) + __builtin_popcountll(high);
        }
#endif
    }

    template <UnsignedIntegral T>
    constexpr bool HasSingleBit(T x) PM_NOEXCEPT
    {
        return PopCount(x) == 1;
    }

    template <UnsignedIntegral T>
    constexpr T BitCeil(T x) PM_NOEXCEPT
    {
        constexpr auto Nd = IntegerTraits<T>::Digits;
        if (x == 0 || x == 1) return 1;
        auto shiftExponent = Nd - CountLeftZero((T)(x - 1u));
        if (!IsConstantEvaluated())
            assert(shiftExponent != IntegerTraits<T>::Digits);

        using PromotedType = decltype(x << 1);
        if constexpr (!IsSameV<PromotedType, T>)
        {
            const i32 extraExponent = sizeof(PromotedType) / sizeof(T) / 2;
            shiftExponent |= (shiftExponent & Nd) << extraExponent;
        }
        return static_cast<T>(1u) << shiftExponent;
    }

    template <UnsignedIntegral T>
    constexpr T BitFloor(T x) PM_NOEXCEPT
    {
        constexpr auto Nd = IntegerTraits<T>::Digits;
        if (x == 0) return 0;
        return (T)1u << (Nd - CountLeftZero((T)(x >> 1)));
    }
    template <UnsignedIntegral T>
    constexpr i32 BitWidth(T x) PM_NOEXCEPT
    {
        constexpr auto Nd = IntegerTraits<T>::Digits;
        return Nd - CountLeftZero(x);
    }

    enum class Endian
    {
        eLittle = __ORDER_LITTLE_ENDIAN__,
        eBig    = __ORDER_BIG_ENDIAN__,
        eNative = __BYTE_ORDER__
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::BitCast;
using Prism::BitCeil;
using Prism::BitFloor;
using Prism::BitWidth;
using Prism::ByteSwap;
using Prism::CountLeftOne;
using Prism::CountLeftZero;
using Prism::CountRightOne;
using Prism::CountRightZero;
using Prism::Endian;
using Prism::HasSingleBit;
using Prism::IntegerTraits;
using Prism::PopCount;
using Prism::RotateLeft;
using Prism::RotateRight;
#endif
