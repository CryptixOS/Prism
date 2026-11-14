/*
 * Created by v1tr10l7 on 05.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Core.Limits;

export import Prism.Core.TypeTraits;

export extern "C++"
{
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
                else return -Max() - 1;
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

        /// bool has two values and one digit; it is not modular.
        template <>
        struct NumericLimits<bool, true, false>
        {
            constexpr static bool IsSpecialized = true;

            constexpr static bool Min() { return false; }
            constexpr static bool Max() { return true; }
            constexpr static bool Lowest() { return false; }

            constexpr static i32  Digits      = 1;
            constexpr static i32  Digits10    = 0;
            constexpr static i32  MaxDigits10 = 0;

            constexpr static bool IsSigned    = false;
            constexpr static bool IsInteger   = true;
            constexpr static bool IsExact     = true;
            constexpr static i32  Radix       = 2;

            constexpr static bool Epsilon() { return false; }
            constexpr static bool RoundError() { return false; }

            constexpr static i32  MinExponent     = 0;
            constexpr static i32  MinExponent10   = 0;
            constexpr static i32  MaxExponent     = 0;
            constexpr static i32  MaxExponent10   = 0;

            constexpr static bool HasInfinity     = false;
            constexpr static bool HasQuietNaN     = false;
            constexpr static bool HasSignalingNaN = false;
            constexpr static FloatDenormStyle HasDenorm
                = FloatDenormStyle::eAbsent;
            constexpr static bool HasDenormLoss = false;

            constexpr static bool Infinity() { return false; }
            constexpr static bool QuietNaN() { return false; }
            constexpr static bool SignalingNaN() { return false; }
            constexpr static bool DenormMin() { return false; }

            constexpr static bool IsIec559       = false;
            constexpr static bool IsBounded      = true;
            constexpr static bool IsModulo       = false;

            constexpr static bool Traps          = false;
            constexpr static bool TinynessBefore = false;
            constexpr static FloatRoundStyle FloatRoundingStyle
                = FloatRoundStyle::eTowardZero;
        };

        /// The compiler's description of each floating-point type.
        template <typename T>
        struct FloatProperties;
        template <>
        struct FloatProperties<float>
        {
            static constexpr float Min        = __FLT_MIN__;
            static constexpr float Max        = __FLT_MAX__;
            static constexpr float Epsilon    = __FLT_EPSILON__;
            static constexpr float DenormMin  = __FLT_DENORM_MIN__;
            static constexpr i32   Digits     = __FLT_MANT_DIG__;
            static constexpr i32   Digits10   = __FLT_DIG__;
            static constexpr i32   MaxDigits10 = __FLT_DECIMAL_DIG__;
            static constexpr i32   MinExp     = __FLT_MIN_EXP__;
            static constexpr i32   MinExp10   = __FLT_MIN_10_EXP__;
            static constexpr i32   MaxExp     = __FLT_MAX_EXP__;
            static constexpr i32   MaxExp10   = __FLT_MAX_10_EXP__;
            static constexpr float Infinity() { return __builtin_inff(); }
            static constexpr float QuietNaN() { return __builtin_nanf(""); }
            static constexpr float SignalingNaN() { return __builtin_nansf(""); }
        };
        template <>
        struct FloatProperties<double>
        {
            static constexpr double Min        = __DBL_MIN__;
            static constexpr double Max        = __DBL_MAX__;
            static constexpr double Epsilon    = __DBL_EPSILON__;
            static constexpr double DenormMin  = __DBL_DENORM_MIN__;
            static constexpr i32    Digits     = __DBL_MANT_DIG__;
            static constexpr i32    Digits10   = __DBL_DIG__;
            static constexpr i32    MaxDigits10 = __DBL_DECIMAL_DIG__;
            static constexpr i32    MinExp     = __DBL_MIN_EXP__;
            static constexpr i32    MinExp10   = __DBL_MIN_10_EXP__;
            static constexpr i32    MaxExp     = __DBL_MAX_EXP__;
            static constexpr i32    MaxExp10   = __DBL_MAX_10_EXP__;
            static constexpr double Infinity() { return __builtin_inf(); }
            static constexpr double QuietNaN() { return __builtin_nan(""); }
            static constexpr double SignalingNaN() { return __builtin_nans(""); }
        };
        template <>
        struct FloatProperties<long double>
        {
            static constexpr long double Min       = __LDBL_MIN__;
            static constexpr long double Max       = __LDBL_MAX__;
            static constexpr long double Epsilon   = __LDBL_EPSILON__;
            static constexpr long double DenormMin = __LDBL_DENORM_MIN__;
            static constexpr i32         Digits    = __LDBL_MANT_DIG__;
            static constexpr i32         Digits10  = __LDBL_DIG__;
            static constexpr i32         MaxDigits10 = __LDBL_DECIMAL_DIG__;
            static constexpr i32         MinExp    = __LDBL_MIN_EXP__;
            static constexpr i32         MinExp10  = __LDBL_MIN_10_EXP__;
            static constexpr i32         MaxExp    = __LDBL_MAX_EXP__;
            static constexpr i32         MaxExp10  = __LDBL_MAX_10_EXP__;
            static constexpr long double Infinity() { return __builtin_infl(); }
            static constexpr long double QuietNaN() { return __builtin_nanl(""); }
            static constexpr long double SignalingNaN()
            {
                return __builtin_nansl("");
            }
        };

        /// Specialization for floating-point types
        template <typename T>
        struct NumericLimits<T, false, true>
        {
            using Properties                    = FloatProperties<RemoveCvType<T>>;
            constexpr static bool IsSpecialized = true;

            constexpr static T    Min() { return Properties::Min; }
            constexpr static T    Max() { return Properties::Max; }
            constexpr static T    Lowest() { return -Max(); }

            constexpr static i32  Digits      = Properties::Digits;
            constexpr static i32  Digits10    = Properties::Digits10;
            constexpr static i32  MaxDigits10 = Properties::MaxDigits10;

            constexpr static bool IsSigned    = true;
            constexpr static bool IsInteger   = false;
            constexpr static bool IsExact     = false;
            constexpr static i32  Radix       = __FLT_RADIX__;

            constexpr static T    Epsilon() { return Properties::Epsilon; }
            constexpr static T    RoundError() { return T(0.5); }

            constexpr static i32  MinExponent     = Properties::MinExp;
            constexpr static i32  MinExponent10   = Properties::MinExp10;
            constexpr static i32  MaxExponent     = Properties::MaxExp;
            constexpr static i32  MaxExponent10   = Properties::MaxExp10;

            constexpr static bool HasInfinity     = true;
            constexpr static bool HasQuietNaN     = true;
            constexpr static bool HasSignalingNaN = true;
            constexpr static FloatDenormStyle HasDenorm
                = FloatDenormStyle::ePresent;
            constexpr static bool HasDenormLoss = false;

            constexpr static T    Infinity() { return Properties::Infinity(); }
            constexpr static T    QuietNaN() { return Properties::QuietNaN(); }
            constexpr static T    SignalingNaN()
            {
                return Properties::SignalingNaN();
            }
            constexpr static T    DenormMin() { return Properties::DenormMin; }

            constexpr static bool IsIec559       = true;
            constexpr static bool IsBounded      = true;
            constexpr static bool IsModulo       = false;

            constexpr static bool Traps          = false;
            constexpr static bool TinynessBefore = false;
            constexpr static FloatRoundStyle FloatRoundingStyle
                = FloatRoundStyle::eToNearest;
        };
    } // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
    using Prism::NumericLimits;
#endif
}
