/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Core/TypeTraits.hpp>

namespace Prism
{
#include <Prism/Details/Concepts.inl>

    template <typename T>
    concept Integral = IsIntegralV<T>;
    template <typename T>
    concept SignedIntegral = Integral<T> && std::is_signed_v<T>;
    template <typename T>
    concept UnsignedIntegral = std::integral<T> && !std::signed_integral<T>;

    template <typename T>
    concept IntegralOrEnum
        = IsIntegralV<T> || IsEnumV<T> || std::is_unsigned_v<T>;
    template <typename T>
    concept PrimitiveOrEnum = IsEnumV<T> || IsArithmeticV<T> || IsSameV<T, bool>
                           || std::is_signed_v<T> || IsIntegralV<T>;

    template <PrimitiveOrEnum T>
    class ArithmeticEnum
    {
      public:
        using ValueType            = PromoteEnumType<T>;

        constexpr ArithmeticEnum() = default;

        constexpr ArithmeticEnum(T&& value)
            : m_Value(value)
        {
        }
        constexpr ArithmeticEnum(const T& value)
            : m_Value(value)
        {
        }
        constexpr ArithmeticEnum(ValueType&& value)
            requires(std::is_enum_v<T>)
            : m_Value(static_cast<T>(value))
        {
        }
        constexpr ArithmeticEnum(const ValueType& value)
            requires(!IsSameV<T, ValueType>)
            : m_Value(static_cast<T>(value))
        {
        }

        constexpr ArithmeticEnum& operator=(T&& value) { m_Value = value; }
        constexpr ArithmeticEnum& operator=(const T& value) { m_Value = value; }

        constexpr                 operator ValueType()
            requires(!IsSameV<T, ValueType>)
        {
            return static_cast<ValueType>(m_Value);
        }
        constexpr      operator T() { return m_Value; }
        constexpr bool operator==(T rhs)
        {
            return m_Value == static_cast<ValueType>(rhs);
        }
        constexpr std::strong_ordering operator<=>(T rhs)
        {
            return m_Value <=> rhs;
        }
        constexpr ArithmeticEnum& operator+=(ValueType v)
        {
            m_Value += v;
            return *this;
        }
        constexpr ArithmeticEnum operator+(ValueType v) const
        {
            return ArithmeticEnum(m_Value + v);
        }

      private:
        T m_Value{};
    };
    template <typename T>
    concept EnumType = IsEnumV<T>;

    template <>
    class ArithmeticEnum<bool>
    {
      public:
        using ValueType            = bool;

        constexpr ArithmeticEnum() = default;
        constexpr ArithmeticEnum(bool value)
            : m_Value(static_cast<ValueType>(value))
        {
        }

        constexpr      operator ValueType() const { return m_Value; }

        constexpr bool operator==(bool rhs)
        {
            return m_Value == static_cast<ValueType>(rhs);
        }
        constexpr std::strong_ordering operator<=>(bool rhs)
        {
            return m_Value <=> static_cast<ValueType>(rhs);
        }

      private:
        ValueType m_Value{};
    };

    template <typename T>
    constexpr bool operator==(ArithmeticEnum<T> lhs, T rhs)
    {
        return lhs == rhs;
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::ArithmeticEnum;
#endif
