/*
 * Created by v1tr10l7 on 12.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    template <bool Signed = true>
    class Integer128
    {
      public:
        using ValueType = ConditionalType<Signed, i64, u64>;

        constexpr Integer128(ValueType high, ValueType low)
            : m_Low(low)
            , m_High(high)
        {
        }
        constexpr Integer128(ValueType value = 0)
            : m_Low(value)
            , m_High(0)
        {
        }

        constexpr ValueType High() const PM_NOEXCEPT { return m_High; }
        constexpr ValueType Low() const PM_NOEXCEPT { return m_Low; }

        constexpr ValueType operator[](usize index) const PM_NOEXCEPT
        {
            return index == 0 ? m_Low : m_High;
        }

        template <typename T, EnableIf<IsIntegral<T>::Value>>
        constexpr explicit operator T() const
        {
            return static_cast<T>(m_Low);
        }

        friend constexpr bool operator==(const Integer128& lhs,
                                         const Integer128& rhs)
        {
            return lhs.m_High == rhs.m_High && lhs.m_Low == rhs.m_Low;
        }
        friend constexpr std::strong_ordering operator<=>(const Integer128& lhs,
                                                          const Integer128& rhs)
        {
            if (lhs.m_High < rhs.m_High) return std::strong_ordering::less;
            if (lhs.m_High > rhs.m_High) return std::strong_ordering::greater;

            if (lhs.m_Low < rhs.m_Low) return std::strong_ordering::less;
            if (lhs.m_Low > rhs.m_Low) return std::strong_ordering::greater;

            return std::strong_ordering::equal;
        }
        friend constexpr Integer128 operator|(const Integer128& lhs,
                                              const Integer128& rhs)
        {
            return {lhs.m_High | rhs.m_High, lhs.m_Low | rhs.m_Low};
        }
        friend constexpr Integer128 operator&(const Integer128& lhs,
                                              const Integer128& rhs)
        {
            return {lhs.m_High & rhs.m_High, lhs.m_Low & rhs.m_Low};
        }
        friend constexpr Integer128 operator~(const Integer128& n)
        {
            return {~n.m_High, ~n.m_Low};
        }
        friend Integer128 operator+(const Integer128& lhs,
                                    const Integer128& rhs)
        {
            auto result = Integer128(lhs);
            result += rhs;
            return result;
        }
        friend Integer128 operator*(const Integer128& lhs, uint32_t rhs)
        {
            assert(lhs.m_High == 0);
            ValueType hi     = (lhs.m_Low >> 32) * rhs;
            ValueType lo     = (lhs.m_Low & ~uint32_t()) * rhs;
            ValueType new_lo = (hi << 32) + lo;
            return {(hi >> 32) + (new_lo < lo ? 1 : 0), new_lo};
        }
        friend Integer128 operator-(const Integer128& lhs, ValueType rhs)
        {
            return {lhs.m_High - (lhs.m_Low < rhs ? 1 : 0), lhs.m_Low - rhs};
        }
        constexpr Integer128 operator>>(int shift) const
        {
            if (shift == 64) return {0, m_High};
            if (shift > 64) return Integer128(0, m_High) >> (shift - 64);
            return {m_High >> shift,
                    (m_High << (64 - shift)) | (m_Low >> shift)};
        }
        constexpr Integer128 operator<<(int shift) const
        {
            if (shift == 64) return {m_Low, 0};
            if (shift > 64) return Integer128(m_Low, 0) << (shift - 64);
            return {m_High << shift | (m_Low >> (64 - shift)),
                    (m_Low << shift)};
        }
        constexpr Integer128& operator>>=(int shift)
        {
            return *this = *this >> shift;
        }
        constexpr void operator+=(Integer128 n)
        {
            ValueType newLow  = m_Low + n.m_Low;
            ValueType newHigh = m_High + n.m_High + (newLow < m_Low ? 1 : 0);
            assert(newHigh >= m_High);
            m_Low  = newLow;
            m_High = newHigh;
        }
        constexpr void operator&=(Integer128 n)
        {
            m_Low &= n.m_Low;
            m_High &= n.m_High;
        }

        constexpr Integer128& operator+=(ValueType n) PM_NOEXCEPT
        {
            if (IsConstantEvaluated())
            {
                m_Low += n;
                m_High += (m_Low < n ? 1 : 0);
                return *this;
            }
#if PrismHasBuiltin(__builtin_addcll)
            unsigned long long carry;
            m_Low = __builtin_addcll(m_Low, n, 0, &carry);
            m_High += carry;
#elif PrismHasBuiltin(__builtin_ia32_addcarryx_u64) 
            unsigned long long result;
            auto carry = __builtin_ia32_addcarryx_u64(0, m_Low, n, &result);
            m_Low      = result;
            m_High += carry;
#else
            m_Low += n;
            m_High += (m_Low < n ? 1 : 0);
#endif
            return *this;
        }

      private:
        ValueType m_Low  = 0;
        ValueType m_High = 0;
    };

    using i128 = Integer128<true>;
    using u128 = Integer128<false>;
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::i128;
using Prism::u128;
#endif
