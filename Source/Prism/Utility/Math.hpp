/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

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
        auto it = ilist.begin();
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
        auto it = ilist.begin();
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
} // namespace Prism::Math

#if PRISM_TARGET_CRYPTIX == 1
namespace Math = Prism::Math;
#endif
