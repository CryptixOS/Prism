/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism::Math
{
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
        return AlignDown(value + alignment - 1, alignment) / alignment;
    }
    constexpr bool IsPowerOfTwo(usize value)
    {
        return value != 0 && !(value & (value - 1));
    }

    template <typename T, typename U>
    constexpr auto RoundUpToPowerOfTwo(T value, U exponent)
        requires(std::is_integral_v<T> && std::is_integral_v<U>)
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
} // namespace Prism::Math

#if PRISM_TARGET_CRYPTIX == 1
namespace Math = Prism::Math;
#endif
