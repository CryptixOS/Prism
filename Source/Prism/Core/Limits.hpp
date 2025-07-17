/*
 * Created by v1tr10l7 on 05.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T>
    struct DigitsImpl
    {
        static constexpr i32 Value = 0;
    };

    // Integral types
    template <typename T>
        requires IsIntegralV<T>
    struct DigitsImpl<T>
    {
        static constexpr i32 Value
            = sizeof(T) * __CHAR_BIT__ - (IsSigned<T>() ? 1 : 0);
    };

#if PRISM_TARGET_CRYPTIX == 0
    // Floating-point types
    template <>
    struct DigitsImpl<float>
    {
        static constexpr i32 Value = __FLT_MANT_DIG__;
    };
    template <>
    struct DigitsImpl<double>
    {
        static constexpr i32 Value = __DBL_MANT_DIG__;
    };
    template <>
    struct DigitsImpl<long double>
    {
        static constexpr i32 Value = __LDBL_MANT_DIG__;
    };
#endif

    template <typename T>
    inline constexpr i32 Digits = DigitsImpl<T>::Value;

    enum class FloatDenormStyle
    {
        eAbsent,
        ePresent,
        eIndeterminate
    };
    enum class FloatRoundStyle
    {
        eTowardZero,
        eToNearest,
        eTowardInfinity,
        eTowardNegInfinity,
        eIndeterminate
    };

    template <typename T, bool = IsIntegral<T>(), bool = IsFloatingPoint<T>()>
    struct NumericLimits;

    /// Generic primary template for unknown types
    template <typename T, bool, bool>
    struct NumericLimits
    {
        constexpr static bool IsSpecialized = false;
    };

    template <typename T>
    inline constexpr i32 Digits10 = Digits<T> * 30103 / 100000;

    /// Specialization for integral types
    template <typename T>
    struct NumericLimits<T, true, false>
    {
        constexpr static bool IsSpecialized = true;

        constexpr static T    Min()
        {
            if constexpr (!IsSignedV<T>) return 0;

            return -Max() - 1;
        }
        constexpr static T Max()
        {
            return static_cast<T>(typename MakeUnsigned<T>::Type(~0)
                                  >> IsSignedV<T>);
        }
        constexpr static T                Lowest() { return Min(); }

        constexpr static i32              Digits      = ::Prism::Digits<T>;
        constexpr static i32              Digits10    = ::Prism::Digits10<T>;
        constexpr static i32              MaxDigits10 = 0;

        constexpr static bool             IsSigned    = IsSignedV<T>;
        constexpr static bool             IsInteger   = true;
        constexpr static bool             IsExact     = true;
        constexpr static i32              Radix       = 2;

        constexpr static T                Epsilon() { return T(0); }
        constexpr static T                RoundError() { return T(0); }

        constexpr static i32              MinExponent     = 0;
        constexpr static i32              MinExponent10   = 0;
        constexpr static i32              MaxExponent     = 0;
        constexpr static i32              MaxExponent10   = 0;

        constexpr static bool             HasInfinity     = false;
        constexpr static bool             HasQuietNaN     = false;
        constexpr static bool             HasSignalingNaN = false;
        constexpr static FloatDenormStyle HasDenorm = FloatDenormStyle::eAbsent;
        constexpr static bool             HasDenormLoss = false;

        constexpr static T                Infinity() { return T(0); }
        constexpr static T                QuietNaN() { return T(0); }
        constexpr static T                SignalingNaN() { return T(0); }
        constexpr static T                DenormMin() { return T(0); }

        constexpr static bool             IsIec559       = false;
        constexpr static bool             IsBounded      = true;
        constexpr static bool             IsModulo       = !IsSigned;

        constexpr static bool             Traps          = true;
        constexpr static bool             TinynessBefore = false;
        constexpr static FloatRoundStyle  FloatRoundingStyle
            = FloatRoundStyle::eTowardZero;
    };

    /// Specialization for floating-point types
    template <typename T>
    struct NumericLimits<T, false, true>
    {
        constexpr static bool IsSpecialized = true;

        constexpr static T    Min()
        {
            return __FLT_MIN__;
        } // Replace with built-in macros per type
        constexpr static T                Max() { return __FLT_MAX__; }
        constexpr static T                Lowest() { return -Max(); }

        constexpr static i32              Digits      = __FLT_MANT_DIG__;
        constexpr static i32              Digits10    = __FLT_DIG__;
        constexpr static i32              MaxDigits10 = __FLT_DECIMAL_DIG__;

        constexpr static bool             IsSigned    = true;
        constexpr static bool             IsInteger   = false;
        constexpr static bool             IsExact     = false;
        constexpr static i32              Radix       = __FLT_RADIX__;

        constexpr static T                Epsilon() { return __FLT_EPSILON__; }
        constexpr static T                RoundError() { return 1.0f; }

        constexpr static i32              MinExponent     = __FLT_MIN_EXP__;
        constexpr static i32              MinExponent10   = __FLT_MIN_10_EXP__;
        constexpr static i32              MaxExponent     = __FLT_MAX_EXP__;
        constexpr static i32              MaxExponent10   = __FLT_MAX_10_EXP__;

        constexpr static bool             HasInfinity     = true;
        constexpr static bool             HasQuietNaN     = true;
        constexpr static bool             HasSignalingNaN = true;
        constexpr static FloatDenormStyle HasDenorm
            = FloatDenormStyle::ePresent;
        constexpr static bool HasDenormLoss = false;

        constexpr static T    Infinity() { return __builtin_inff(); }
        constexpr static T    QuietNaN() { return __builtin_nanf(""); }
        constexpr static T    SignalingNaN() { return __builtin_nansf(""); }
        constexpr static T    DenormMin() { return __FLT_DENORM_MIN__; }

        constexpr static bool IsIec559       = true;
        constexpr static bool IsBounded      = true;
        constexpr static bool IsModulo       = false;

        constexpr static bool Traps          = false;
        constexpr static bool TinynessBefore = true;
        constexpr static FloatRoundStyle FloatRoundingStyle
            = FloatRoundStyle::eToNearest;
    };
} // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::NumericLimits;
#endif
