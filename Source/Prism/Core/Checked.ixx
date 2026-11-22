/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Core.Checked;

export import Prism.Core.Limits;
export import Prism.Core.TypeTraits;

/**
 * Overflow-checked and saturating integer arithmetic.
 *
 *  - Checked<T> carries an overflow flag through a calculation:
 *        Checked<usize> bytes = count;
 *        bytes *= sizeof(T);
 *        bytes += header;
 *        if (bytes.HasOverflow()) return Error(ENOMEM);
 *  - CheckedAdd/Sub/Mul(a, b, out) return false on overflow.
 *  - SaturatingAdd/Sub/Mul clamp to the limits of T.
 *  - AllocationSize<T>(count) is count * sizeof(T), and a PrismCheck failure
 *    (a panic in hardened builds) when that overflows. Containers use it for
 *    every allocation.
 */

export extern "C++"
{
    namespace Prism
    {
        template <typename T>
            requires IsIntegralV<T>
        PM_NODISCARD constexpr bool CheckedAdd(T a, T b, T& out)
        {
            return !__builtin_add_overflow(a, b, &out);
        }
        template <typename T>
            requires IsIntegralV<T>
        PM_NODISCARD constexpr bool CheckedSub(T a, T b, T& out)
        {
            return !__builtin_sub_overflow(a, b, &out);
        }
        template <typename T>
            requires IsIntegralV<T>
        PM_NODISCARD constexpr bool CheckedMul(T a, T b, T& out)
        {
            return !__builtin_mul_overflow(a, b, &out);
        }

        template <typename T>
            requires IsIntegralV<T>
        PM_NODISCARD constexpr T SaturatingAdd(T a, T b)
        {
            T result;
            if (CheckedAdd(a, b, result)) return result;
            // Only same-signed operands overflow, in the direction of b.
            return b < T(0) ? NumericLimits<T>::Min() : NumericLimits<T>::Max();
        }
        template <typename T>
            requires IsIntegralV<T>
        PM_NODISCARD constexpr T SaturatingSub(T a, T b)
        {
            T result;
            if (CheckedSub(a, b, result)) return result;
            return b > T(0) ? NumericLimits<T>::Min() : NumericLimits<T>::Max();
        }
        template <typename T>
            requires IsIntegralV<T>
        PM_NODISCARD constexpr T SaturatingMul(T a, T b)
        {
            T result;
            if (CheckedMul(a, b, result)) return result;
            return (a < T(0)) != (b < T(0)) ? NumericLimits<T>::Min()
                                            : NumericLimits<T>::Max();
        }

        /**
         * @brief An integer that remembers whether any operation on it
         * overflowed. Once overflowed, it stays overflowed.
         */
        template <typename T>
            requires IsIntegralV<T>
        class Checked
        {
          public:
            constexpr Checked() = default;
            constexpr Checked(T value)
                : m_Value(value)
            {
            }

            PM_NODISCARD constexpr bool HasOverflow() const { return m_Overflow; }
            /// The value; only meaningful without overflow (it is checked).
            PM_NODISCARD constexpr T    Value() const
            {
                PrismCheck(!m_Overflow);
                return m_Value;
            }
            /// The value if there was no overflow, fallback otherwise.
            PM_NODISCARD constexpr T ValueOr(T fallback) const
            {
                return m_Overflow ? fallback : m_Value;
            }

            constexpr Checked& operator+=(Checked other)
            {
                m_Overflow |= other.m_Overflow
                            || !CheckedAdd(m_Value, other.m_Value, m_Value);
                return *this;
            }
            constexpr Checked& operator-=(Checked other)
            {
                m_Overflow |= other.m_Overflow
                            || !CheckedSub(m_Value, other.m_Value, m_Value);
                return *this;
            }
            constexpr Checked& operator*=(Checked other)
            {
                m_Overflow |= other.m_Overflow
                            || !CheckedMul(m_Value, other.m_Value, m_Value);
                return *this;
            }
            constexpr Checked& operator/=(Checked other)
            {
                if (other.m_Overflow || other.m_Value == 0
                    || (IsSignedV<T> && m_Value == NumericLimits<T>::Min()
                        && other.m_Value == T(-1)))
                    m_Overflow = true;
                else m_Value /= other.m_Value;
                return *this;
            }

            friend constexpr Checked operator+(Checked a, Checked b)
            {
                return a += b;
            }
            friend constexpr Checked operator-(Checked a, Checked b)
            {
                return a -= b;
            }
            friend constexpr Checked operator*(Checked a, Checked b)
            {
                return a *= b;
            }
            friend constexpr Checked operator/(Checked a, Checked b)
            {
                return a /= b;
            }
            friend constexpr bool operator==(Checked a, Checked b)
            {
                return a.m_Overflow == b.m_Overflow && a.m_Value == b.m_Value;
            }

          private:
            T    m_Value    = 0;
            bool m_Overflow = false;
        };

        /// count * sizeof(T), checked: a size that doesn't fit is a
        /// PrismCheck failure, never a short allocation.
        template <typename T>
        PM_NODISCARD constexpr usize AllocationSize(usize count)
        {
            usize bytes = 0;
            bool  fits  = CheckedMul(count, sizeof(T), bytes);
            PrismCheck(fits);
            return bytes;
        }
        /// The capacity to grow to when `required` elements don't fit into
        /// `capacity`: at least double, at least `required`, checked.
        PM_NODISCARD constexpr usize GrowCapacity(usize capacity, usize required,
                                                  usize minimum = 1)
        {
            usize doubled = SaturatingMul<usize>(capacity, 2);
            usize result  = doubled > required ? doubled : required;
            return result > minimum ? result : minimum;
        }
    } // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::Checked;
    using Prism::CheckedAdd;
    using Prism::CheckedMul;
    using Prism::CheckedSub;
    using Prism::SaturatingAdd;
    using Prism::SaturatingMul;
    using Prism::SaturatingSub;
#endif
}
