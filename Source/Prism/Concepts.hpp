/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/TypeTraits.hpp>

namespace Prism
{
#include <Prism/Details/Concepts.inl>

    template <typename T>
    concept Integral = std::is_integral_v<T>;
    template <typename T>
    concept SignedIntegral = std::integral<T> && std::is_signed_v<T>;
    template <typename T>
    concept UnsignedIntegral = std::integral<T> && !std::signed_integral<T>;

    template <typename T>
    concept IntegralOrEnum = IsIntegral<T> || IsEnum<T>;

    template <IntegralOrEnum T>
    class ArithmethicEnum
    {
      public:
        using ValueType = PromoteEnumType<T>;
        static_assert(sizeof(T) == sizeof(ValueType));

        constexpr ArithmethicEnum() = default;
        constexpr ArithmethicEnum(T value)
            : m_Value(static_cast<ValueType>(value))
        {
        }
        constexpr ArithmethicEnum(ValueType value)
            : m_Value(value)
        {
        }

        constexpr      operator ValueType() const { return m_Value; }

        constexpr bool operator==(T rhs)
        {
            return m_Value == static_cast<ValueType>(rhs);
        }
        constexpr std::strong_ordering operator<=>(T rhs)
        {
            return m_Value <=> rhs;
        }
        constexpr ArithmethicEnum& operator+=(ValueType v)
        {
            m_Value += v;
            return *this;
        }
        constexpr ArithmethicEnum operator+(ValueType v) const
        {
            return ArithmethicEnum(m_Value + v);
        }

      private:
        ValueType m_Value{};
    };

    template <typename T>
    constexpr bool operator==(ArithmethicEnum<T> lhs, T rhs)
    {
        return lhs == rhs;
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::ArithmethicEnum;
#endif
