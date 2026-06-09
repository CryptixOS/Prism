/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Core/Core.hpp>

#if PRISM_TARGET_CRYPTIX == 0
    #include <math.h>
#endif

namespace Prism
{
    namespace Math
    {
        using Prism::Max;
        using Prism::Min;

        /**
         * @brief Computes the absolute value of an integral type safely,
         * returning an unsigned type.
         *
         * This prevents undefined behavior associated with computing the
         * absolute value of the minimum representable value of a signed integer
         * type (e.g., INT_MIN).
         *
         * @tparam T An integral type.
         * @tparam UT The corresponding unsigned integral type (deduced
         * automatically).
         * @param value The input integer value.
         * @return The absolute value as an unsigned integer.
         */
        template <Integral T, UnsignedIntegral UT = MakeUnsigned<T>::Type>
        constexpr UT Absolute(T value)
        {
            return static_cast<UT>(value < 0 ? -value : value);
        }
#if PRISM_TARGET_CRYPTIX == 0
        /**
         * @brief Computes the absolute value of a floating-point type.
         *
         * Excluded on Cryptix target environments due to platform math
         * restrictions.
         *
         * @tparam T A floating-point type.
         * @param value The input floating-point value.
         * @return The absolute value of the input.
         */
        template <FloatingPoint T>
        constexpr T Absolute(T value)
        {
            return value < 0 ? value * -1 : value;
        }
#endif

        /**
         * @brief Clamps a value between an upper and lower bound using default
         * comparison.
         *
         * If the value is less than the lower bound, returns the lower bound.
         * If the value is greater than the upper bound, returns the upper
         * bound. Otherwise, returns a reference to the value itself.
         *
         * @tparam T The type of the values to compare.
         * @param value The value to be clamped.
         * @param low The lower threshold boundary.
         * @param high The upper threshold boundary.
         * @return A constant reference to the clamped value.
         */
        template <typename T>
        constexpr const T& Clamp(const T& value, const T& low, const T& high)
        {
            return Min(Max(value, low), high);
        }
        /**
         * @brief Clamps a value between an upper and lower bound using a custom
         * comparator.
         *
         * Evaluates the boundaries using the provided binary comparison
         * functor.
         *
         * @tparam T The type of the values to compare.
         * @tparam Compare The type of the custom comparator.
         * @param value The value to be clamped.
         * @param low The lower threshold boundary.
         * @param high The upper threshold boundary.
         * @param comp A binary predicate that returns true if the first
         * argument is less than the second.
         * @return A constant reference to the clamped value.
         */
        template <typename T, typename Compare>
        constexpr const T& Clamp(const T& value, const T& low, const T& high,
                                 Compare comp)
        {
            return comp(value, low) ? low : comp(high, value) ? high : value;
        }
        /**
         * @brief Aligns a value down to the nearest multiple of the specified
         * alignment.
         *
         * @note The alignment parameter must be a power of two.
         *
         * @param value The value to align.
         * @param alignment The alignment boundary (must be a power of two).
         * @return The aligned value.
         */
        constexpr usize AlignDown(usize value, usize alignment)
        {
            return value & ~(alignment - 1);
        }
        /**
         * @brief Aligns a value up to the nearest multiple of the specified
         * alignment.
         *
         * @note The alignment parameter must be a power of two.
         *
         * @param value The value to align.
         * @param alignment The alignment boundary (must be a power of two).
         * @return The aligned value.
         */
        constexpr usize AlignUp(usize value, usize alignment)
        {
            return AlignDown(value + alignment - 1, alignment);
        }
        /**
         * @brief Performs integer division, rounding the fractional result up
         * to the next integer.
         *
         * Commonly used to calculate required buffer allocations or thread
         * group counts.
         *
         * @param value The dividend.
         * @param alignment The divisor.
         * @return The result of the division rounded up.
         */
        constexpr auto DivRoundUp(auto value, auto alignment)
        {
            return (value + alignment - 1) / alignment;
        }
        /**
         * @brief Checks whether an unsigned integer is a valid power of two.
         *
         * @param value The value to check.
         * @return True if the value is a power of two, false otherwise
         * (including 0).
         */
        constexpr bool IsPowerOfTwo(usize value)
        {
            return value != 0 && !(value & (value - 1));
        }
        /**
         * @brief Rounds a value up to the next power of two using a dedicated
         * exponent boundary multiplier.
         *
         * @tparam T An integral type for the base value.
         * @tparam U An integral type for the exponent expression.
         * @param value The input value.
         * @param exponent The alignment multiplier value.
         * @return The value rounded up using bitwise mask adjustments.
         */
        template <typename T, typename U>
        constexpr auto RoundUpToPowerOfTwo(T value, U exponent)
            requires(IsIntegralV<T> && IsIntegralV<U>)
        {
            return ((value - 1) & ~(exponent - 1)) + exponent;
        }
        /**
         * @brief Computes the value of a base raised to an integer exponent.
         *
         * Handles both positive and negative integer exponents.
         *
         * @tparam T The numeric type of the base and exponent.
         * @param base The base value.
         * @param exponent The power value.
         * @return The calculated power result.
         */
        template <typename T>
        inline T PowerOf(T base, T exponent)
        {
            T invert  = 0;
            T powMult = 1;

            if ((invert = (exponent < 0))) exponent = -exponent;
            for (T x = 0; x < exponent; x++) powMult *= base;

            return (invert) ? 1 / powMult : powMult;
        }
        /**
         * @brief Calculates the integer square root of a signed integer using a
         * bitwise digit-by-digit algorithm.
         *
         * @param n The input integer.
         * @return The integer square root, or -1 if the input is negative.
         */
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
        /**
         * @brief Computes the base-2 logarithm of an integer using De Bruijn
         * bit-multiplication sequence lookup tables.
         *
         * Highly optimized approach bypassing traditional loops or hardware
         * instructions.
         *
         * @tparam T An integral numeric type.
         * @param value The input value.
         * @return The floor integer log2 calculation.
         */
        template <typename T>
        constexpr T Log2(const T value)
        {
            const auto lookupTable = ToArray<i32>(
                {63, 0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54,
                 33, 42, 3,  61, 51, 37, 40, 49, 18, 28, 20, 55, 30,
                 34, 11, 43, 14, 22, 4,  62, 57, 46, 52, 38, 26, 32,
                 41, 50, 36, 17, 19, 29, 10, 13, 21, 56, 45, 25, 31,
                 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5});

            T result = value;

            for (usize i = 1; i <= 32; i = i * 2) result |= result >> i;

            constexpr usize MAGIC       = 0x07edd5e59a4e28c2;
            constexpr usize SHIFT_WIDTH = 58;
            auto index = static_cast<u64>((result - (result >> 1)) * MAGIC)
                      >> SHIFT_WIDTH;
            return lookupTable[index];
        }
        /**
         * @brief Explicitly requests compile-time evaluation of the Log2
         * algorithm.
         *
         * @tparam T An integral type.
         * @param value The constant expression value.
         * @return The computed log2 evaluated at compile-time.
         */
        template <typename T>
        consteval T Log2CEval(const T value)
        {
            return Log2(value);
        }

#if PRISM_TARGET_CRYPTIX == 0
        /**
         * @brief Calculates the square root of a floating-point type using
         * architectural assembly or compiler builtins.
         *
         * Uses inline assembly extensions for x86_64 or AArch64 targets during
         * runtime, and fallbacks to compiler intrinsics during constant
         * evaluation contexts.
         *
         * @tparam T Floating-point variant type.
         * @param value The value to pass.
         * @return Calculated square root.
         */
        template <typename T>
        constexpr T SquareRoot(T value)
        {
            if (IsConstantEvaluated())
            {
                if (IsSameV<T, long double>) return __builtin_sqrtl(value);
                if (IsSameV<T, f64>) return __builtin_sqrt(value);
                if (IsSameV<T, f32>) return __builtin_sqrtf(value);
            }

    #ifdef PRISM_TARGET_X86_64
            if constexpr (IsSame<T, f32>)
            {
                f32 res;
                __asm__ volatile("sqrtss %1, %0" : "=x"(res) : "x"(value));
                return res;
            }
            if constexpr (IsSame<T, f64>)
            {
                f64 res;
                __asm__ volatile("sqrtsd %1, %0" : "=x"(res) : "x"(value));
                return res;
            }

            T res;
            __asm__ volatile("fsqrt" : "=t"(res) : "0"(value));
            return res;
    #elifdef PRISM_TARGET_AARCH64
            if constexpr (IsSameV<T, long double>) PrismToDo();
            if constexpr (IsSameV<T, f64>)
            {
                f64 res;
                __asm__ volatile("fsqrt %d0, %d1" : "=w"(res) : "w"(value));
                return res;
            }
            if constexpr (IsSameV<T, f32>)
            {
                f32 res;
                __asm__ volatile(#instruction " %s0, %s1"
                                 : "=w"(res)
                                 : "w"(value));
                return res;
            }

    #else
            return __builtin_sqrt(value);
    #endif
        }
#endif

#if PRISM_TARGET_CRYPTIX == 0
        /**
         * @brief Computes the smallest integer value not less than the input
         * argument.
         *
         * @tparam T A floating-point type.
         * @param value The input floating-point value.
         * @return The ceiling value of the input.
         */
        template <FloatingPoint T>
        constexpr T Ceil(T value)
        {
            if (IsConstantEvaluated())
            {
                if (value < NumericLimits<i64>::Min()
                    || value > NumericLimits<i64>::Max())
                    return value;
                return (static_cast<T>(static_cast<i64>(value)) == value)
                         ? static_cast<i64>(value)
                         : static_cast<i64>(value) + ((value > 0) ? 1 : 0);
            }
    #ifdef PRISM_TARGET_AARCH64
            T res;

            if constexpr (IsSameV<T, long double>) TODO();
            else if constexpr (IsSameV<T, double>)
            {
                __asm__ volatile("frintp %d0, %d1" : "=w"(res) : "w"(value));
                return res;
            }
            if constexpr (IsSameV<T, float>)
            {
                __asm__ volatile("frintp %s0, %s1" : "=w"(res) : "w"(value));
                return res;
            }
    #else
            if constexpr (IsSameV<T, long double>)
                return __builtin_ceill(value);
            if constexpr (IsSameV<T, double>) return __builtin_ceil(value);
            if constexpr (IsSameV<T, float>) return __builtin_ceilf(value);
    #endif
        }
        /**
         * @brief Computes the largest integer value not greater than the input
         * argument.
         *
         * @tparam T A floating-point type.
         * @param value The input floating-point value.
         * @return The floor value of the input.
         */
        template <FloatingPoint T>
        constexpr T Floor(T value)
        {
            if (IsConstantEvaluated())
            {
                if (value < NumericLimits<i64>::Min()
                    || value > NumericLimits<i64>::Max())
                    return value;
                return (static_cast<T>(static_cast<i64>(value)) == value)
                         ? static_cast<i64>(value)
                         : static_cast<i64>(value) - ((value > 0) ? 0 : 1);
            }
    #ifdef PRISM_TARGET_AARCH64
            T res;

            if constexpr (IsSameV<T, long double>) TODO();
            if constexpr (IsSameV<T, double>)
            {
                __asm__ volatile("frintm %d0, %d1" : "=w"(res) : "w"(value));
                return res;
            }
            if constexpr (IsSameV<T, float>)
            {
                __asm__ volatile("frintm %s0, %s1" : "=w"(res) : "w"(value));
                return res;
            }
    #else
            if constexpr (IsSameV<T, long double>)
                return __builtin_floorl(value);
            if constexpr (IsSameV<T, double>) return __builtin_floor(value);
            if constexpr (IsSameV<T, float>) return __builtin_floorf(value);
    #endif
        }

        /**
         * @brief Rounds a numerical value to the nearest integer, rounding
         * halfway cases away from zero.
         *
         * This function is overloaded via templates to handle integers,
         * floating-point numbers, and custom numeric types efficiently.
         *
         * @tparam T Any numeric type (floating-point or integral).
         * @param value The value to round.
         * @return The rounded value matching the input type.
         */
        template <typename T>
        PM_NODISCARD constexpr T Round(T value) PM_NOEXCEPT
        {
            if (IsConstantEvaluated())
            {
                if constexpr (IsSameV<T, double>) return __builtin_round(value);
                else if constexpr (IsSameV<T, float>)
                    return __builtin_roundf(value);
                else if constexpr (IsSameV<T, long double>)
                    return __builtin_roundl(value);
            }

            if constexpr (IsIntegralV<T>) return value;

            if (value == 0.) return value;
            if (value > 0.) return Floor(value + .5);
            return Ceil(value - .5);
        }
#endif
    }; // namespace Math
} // namespace Prism

#if PRISM_USE_NAMESPACE != 0
namespace Math = Prism::Math;
#endif
