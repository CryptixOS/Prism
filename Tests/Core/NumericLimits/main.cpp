/*
 * Created by v1tr10l7 on 17.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <cstdint>
#include <limits>
#include <type_traits>

#include <Prism/Core/Limits.hpp>

using namespace Prism;

template <typename T>
constexpr void TestIntegralLimits()
{
    static_assert(NumericLimits<T>::IsSpecialized
                  == std::numeric_limits<T>::is_specialized);
    static_assert(NumericLimits<T>::Min() == std::numeric_limits<T>::min());
    static_assert(NumericLimits<T>::Max() == std::numeric_limits<T>::max());
    static_assert(NumericLimits<T>::Lowest()
                  == std::numeric_limits<T>::lowest());

    static_assert(NumericLimits<T>::Digits == std::numeric_limits<T>::digits);
    static_assert(NumericLimits<T>::Digits10
                  == std::numeric_limits<T>::digits10);
    static_assert(NumericLimits<T>::IsSigned
                  == std::numeric_limits<T>::is_signed);
    static_assert(NumericLimits<T>::IsInteger
                  == std::numeric_limits<T>::is_integer);
    static_assert(NumericLimits<T>::IsExact
                  == std::numeric_limits<T>::is_exact);
    static_assert(NumericLimits<T>::Radix == std::numeric_limits<T>::radix);

    static_assert(NumericLimits<T>::IsBounded
                  == std::numeric_limits<T>::is_bounded);
    static_assert(NumericLimits<T>::IsModulo
                  == std::numeric_limits<T>::is_modulo);
}

template <typename T>
constexpr void TestFloatLimits()
{
    static_assert(NumericLimits<T>::IsSpecialized
                  == std::numeric_limits<T>::is_specialized);
    static_assert(NumericLimits<T>::Min() == std::numeric_limits<T>::min());
    static_assert(NumericLimits<T>::Max() == std::numeric_limits<T>::max());
    static_assert(NumericLimits<T>::Lowest()
                  == std::numeric_limits<T>::lowest());

    static_assert(NumericLimits<T>::Digits == std::numeric_limits<T>::digits);
    static_assert(NumericLimits<T>::Digits10
                  == std::numeric_limits<T>::digits10);
    static_assert(NumericLimits<T>::MaxDigits10
                  == std::numeric_limits<T>::max_digits10);
    static_assert(NumericLimits<T>::IsSigned
                  == std::numeric_limits<T>::is_signed);
    static_assert(NumericLimits<T>::IsInteger
                  == std::numeric_limits<T>::is_integer);
    static_assert(NumericLimits<T>::IsExact
                  == std::numeric_limits<T>::is_exact);
    static_assert(NumericLimits<T>::Radix == std::numeric_limits<T>::radix);

    static_assert(NumericLimits<T>::Epsilon()
                  == std::numeric_limits<T>::epsilon());
    static_assert(NumericLimits<T>::RoundError()
                  == std::numeric_limits<T>::round_error());

    static_assert(NumericLimits<T>::MinExponent
                  == std::numeric_limits<T>::min_exponent);
    static_assert(NumericLimits<T>::MinExponent10
                  == std::numeric_limits<T>::min_exponent10);
    static_assert(NumericLimits<T>::MaxExponent
                  == std::numeric_limits<T>::max_exponent);
    static_assert(NumericLimits<T>::MaxExponent10
                  == std::numeric_limits<T>::max_exponent10);

    static_assert(NumericLimits<T>::HasInfinity
                  == std::numeric_limits<T>::has_infinity);
    static_assert(NumericLimits<T>::HasQuietNaN
                  == std::numeric_limits<T>::has_quiet_NaN);
    static_assert(NumericLimits<T>::HasSignalingNaN
                  == std::numeric_limits<T>::has_signaling_NaN);
    static_assert(static_cast<int>(NumericLimits<T>::HasDenorm)
                  == static_cast<int>(std::numeric_limits<T>::has_denorm));
    static_assert(NumericLimits<T>::HasDenormLoss
                  == std::numeric_limits<T>::has_denorm_loss);

    static_assert(NumericLimits<T>::Infinity()
                  == std::numeric_limits<T>::infinity());
    static_assert(NumericLimits<T>::QuietNaN()
                  != NumericLimits<T>::QuietNaN()); // NaN != NaN
    static_assert(NumericLimits<T>::SignalingNaN()
                  != NumericLimits<T>::SignalingNaN());
    static_assert(NumericLimits<T>::DenormMin()
                  == std::numeric_limits<T>::denorm_min());

    static_assert(NumericLimits<T>::IsIec559
                  == std::numeric_limits<T>::is_iec559);
    static_assert(NumericLimits<T>::IsBounded
                  == std::numeric_limits<T>::is_bounded);
    static_assert(NumericLimits<T>::IsModulo
                  == std::numeric_limits<T>::is_modulo);

    static_assert(NumericLimits<T>::Traps == std::numeric_limits<T>::traps);
    static_assert(NumericLimits<T>::TinynessBefore
                  == std::numeric_limits<T>::tinyness_before);
    static_assert(static_cast<int>(NumericLimits<T>::FloatRoundingStyle)
                  == static_cast<int>(std::numeric_limits<T>::round_style));
}

// Run all tests at compile time
constexpr void RunAllTests()
{
    // Integral types
    TestIntegralLimits<bool>();
    TestIntegralLimits<char>();
    TestIntegralLimits<signed char>();
    TestIntegralLimits<unsigned char>();
    TestIntegralLimits<short>();
    TestIntegralLimits<unsigned short>();
    TestIntegralLimits<int>();
    TestIntegralLimits<unsigned int>();
    TestIntegralLimits<long>();
    TestIntegralLimits<unsigned long>();
    TestIntegralLimits<long long>();
    TestIntegralLimits<unsigned long long>();
    TestIntegralLimits<std::int8_t>();
    TestIntegralLimits<std::uint8_t>();
    TestIntegralLimits<std::int16_t>();
    TestIntegralLimits<std::uint16_t>();
    TestIntegralLimits<std::int32_t>();
    TestIntegralLimits<std::uint32_t>();
    TestIntegralLimits<std::int64_t>();
    TestIntegralLimits<std::uint64_t>();

#if PRISM_TARGET_CRYPTIX == 0
    // Floating-point types
    TestFloatLimits<float>();
    TestFloatLimits<double>();
    TestFloatLimits<long double>();
#endif
}

// Entry point for build
constexpr int _static_asserts_trigger = (RunAllTests(), 0);
