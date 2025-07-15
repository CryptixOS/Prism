/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Core/Core.hpp>

namespace Prism::Math
{
    template <typename T>
    constexpr const T& Min(const T& lhs, const T& rhs)
    {
        return lhs < rhs ? lhs : rhs;
    }
    template <typename T>
    constexpr const T& Min(std::initializer_list<T> ilist)
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
    constexpr const T& Max(std::initializer_list<T> ilist)
    {
        auto     it     = ilist.begin();
        const T& result = *it++;
        for (; it != ilist.end(); it++)
            if (*it > result) result = *it;

        return result;
    }

    template <typename T, typename UT = MakeUnsigned<T>::Type>
    constexpr UT Absolute(T value)
    {
        return static_cast<UT>(value < 0 ? -value : value);
    }
    constexpr usize AlignDown(usize value, usize alignment)
    {
        return value & ~(alignment - 1);
    }
    constexpr usize AlignUp(usize value, usize alignment)
    {
        return AlignDown(value + alignment - 1, alignment);
    }
    constexpr auto DivRoundUp(auto value, auto alignment)
    {
        return (value + alignment - 1) / alignment;
    }
    constexpr bool IsPowerOfTwo(usize value)
    {
        return value != 0 && !(value & (value - 1));
    }

    template <typename T, typename U>
    constexpr auto RoundUpToPowerOfTwo(T value, U exponent)
        requires(IsIntegralV<T> && IsIntegralV<U>)
    {
        return ((value - 1) & ~(exponent - 1)) + exponent;
    }

    template <typename T>
    inline T PowerOf(T base, T exponent)
    {
        T invert  = 0;
        T powMult = 1;

        if ((invert = (exponent < 0))) exponent = -exponent;
        for (T x = 0; x < exponent; x++) powMult *= base;

        return (invert) ? 1 / powMult : powMult;
    }
    constexpr isize Sqrt(isize n)
    {
        if (n < 0) return -1;
        i64 result = 0;
        i64 bit    = 1 << 30;
        while (bit > n) bit >>= 2;

        while (bit != 0)
        {
            if (n >= result + bit)
            {
                n -= result + bit;
                result = (result >> 1) + bit;
            }
            else result >>= 1;

            bit >>= 2;
        }

        return result;
    }

    template <typename T>
    constexpr T Log2(const T value)
    {
        const auto lookupTable = ToArray<i32>(
            {63, 0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54, 33, 42, 3,
             61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4,
             62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21,
             56, 45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5});

        T result = value;

        for (usize i = 1; i <= 32; i = i * 2)
            result |= result >> i;

        constexpr usize MAGIC       = 0x07edd5e59a4e28c2;
        constexpr usize SHIFT_WIDTH = 58;
        auto index = static_cast<u64>((result - (result >> 1)) * MAGIC) >> SHIFT_WIDTH;
        return lookupTable[index];
    }
    template <typename T>
    consteval T Log2CEval(const T value)
    {
        return Log2(value);
    }
} // namespace Prism::Math

#if PRISM_TARGET_CRYPTIX == 1
namespace Math = Prism::Math;
#endif
