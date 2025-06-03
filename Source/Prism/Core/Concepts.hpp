/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    namespace Details
    {
        template <typename T, typename U>
        concept SameHelper = IsSameV<T, U>;
    }

    template <typename T, typename U>
    concept SameAs = Details::SameHelper<T, U> && Details::SameHelper<U, T>;
    template <typename Derived, typename Base>
    concept DerivedFrom
        = IsBaseOf<Base, Derived>::Value
       && IsConvertible<const volatile Derived*, const volatile Base*>::Value;
    template <typename From, typename To>
    concept ConvertibleTo = IsConvertible<From, To>::Value
                         && requires { static_cast<To>(DeclVal<From>()); };
    // template <typename T, typename U>
    // concept CommonReferenceWith
    //     = SameAs<CommonReferenceType<T, U>, CommonReferenceType<U, T>>
    //    && ConvertibleTo<T, CommonReferenceType<T, U>>
    //    && ConvertibleTo<U, CommonReferenceType<T, U>>;

    template <typename T>
    concept Integral = IsIntegralV<T>;
    template <typename T>
    concept SignedIntegral = Integral<T> && IsSignedV<T>;
    template <typename T>
    concept UnsignedIntegral = Integral<T> && !SignedIntegral<T>;
#if PRISM_TARGET_CRYPTIX == 0
    template <typename T>
    concept FloatingPoint = IsFloatingPointV<T>;
#endif

    template <typename T>
    concept IntegralOrEnum = IsIntegralV<T> || IsEnumV<T> || IsUnsignedV<T>;
    template <typename T>
    concept PrimitiveOrEnum = IsEnumV<T> || IsArithmeticV<T> || IsSameV<T, bool>
                           || IsSignedV<T> || IsIntegralV<T>;

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
            requires(IsEnumV<T>)
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

    template <typename T>
    concept IsImplicitlyDefaultConstructible
        = IsDefaultConstructibleV<T> && IsTriviallyDefaultConstructibleV<T>;
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::ArithmeticEnum;
#endif
