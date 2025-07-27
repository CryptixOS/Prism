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

        template <typename T>
        using CRef = const RemoveReferenceType<T>&;

        template <typename T>
        concept ClassOrEnum = IsClassV<T> || IsUnionV<T> || IsEnumV<T>;

        template <typename T>
        constexpr bool DestructibleImpl = false;
        template <typename T>
            requires requires(T& t) {
                { t.~T() }
                PM_NOEXCEPT;
            }
        constexpr bool DestructibleImpl<T> = true;

        template <typename T>
        constexpr bool Destructible = DestructibleImpl<T>;
        template <typename T>
        constexpr bool Destructible<T&> = true;
        template <typename T>
        constexpr bool Destructible<T&&> = true;
        template <typename T, usize N>
        constexpr bool Destructible<T[N]> = Destructible<T>;
    } // namespace Details

    template <typename T, typename U>
    concept SameAs = Details::SameHelper<T, U> && Details::SameHelper<U, T>;
    template <typename Derived, typename Base>
    concept DerivedFrom
        = IsBaseOf<Base, Derived>::Value
       && IsConvertible<const volatile Derived*, const volatile Base*>::Value;
    template <typename From, typename To>
    concept ConvertibleTo = IsConvertibleV<From, To>
                         && requires { static_cast<To>(DeclVal<From>()); };
    template <typename T, typename U>
    concept CommonReferenceWith
        = SameAs<CommonReferenceType<T, U>, CommonReferenceType<U, T>>
       && ConvertibleTo<T, CommonReferenceType<T, U>>
       && ConvertibleTo<U, CommonReferenceType<T, U>>;
    template <typename T, typename U>
    concept CommonWith = SameAs<CommonTypeType<T, U>, CommonTypeType<U, T>> && requires {
        static_cast<CommonTypeType<T, U>>(DeclVal<T>());
        static_cast<CommonTypeType<T, U>>(DeclVal<U>());
    } && CommonReferenceWith<AddLValueReferenceType<const T>, AddLValueReferenceType<const U>> && CommonReferenceWith<AddLValueReferenceType<CommonTypeType<T, U>>, CommonReferenceType<AddLValueReferenceType<const T>, AddLValueReferenceType<const U>>>;

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
    template <typename LHS, typename RHS>
    concept AssignableFrom
        = IsLValueReferenceV<LHS>
       && CommonReferenceWith<const RemoveReferenceType<LHS>&,
                              const RemoveReferenceType<RHS>&>
       && requires(LHS lhs, RHS&& rhs) {
              { lhs = Forward<RHS>(rhs) } -> SameAs<LHS>;
          };
    template <typename T>
    concept Swappable = requires(T& a, T& b) { Swap(a, b); };

    namespace Ranges
    {

    };

    template <typename T>
    concept Destructible = Details::Destructible<T>;
    template <typename T, typename... Args>
    concept ConstructibleFrom = Destructible<T> && IsConstructibleV<T, Args...>;
    template <typename T>
    concept DefaultInitializable = ConstructibleFrom<T> && requires {
        T{};
        ::new T;
    };
    template <typename T>
    concept MoveConstructible = ConstructibleFrom<T, T> && ConvertibleTo<T, T>;
    template <class T>
    concept CopyConstructible
        = MoveConstructible<T> && ConstructibleFrom<T, T&>
       && ConvertibleTo<T&, T> && ConstructibleFrom<T, const T&>
       && ConvertibleTo<const T&, T> && ConstructibleFrom<T, const T>
       && ConvertibleTo<const T, T>;

    template <typename T>
    concept BooleanTestableImpl = ConvertibleTo<T, bool>;
    template <typename T>
    concept BooleanTestable = BooleanTestableImpl<T> && requires(T&& t) {
        { !Forward<T>(t) } -> BooleanTestableImpl;
    };

    template <typename T>
    concept Movable = IsObjectV<T> && MoveConstructible<T>
                   && AssignableFrom<T&, T> && Swappable<T>;
    template <typename T>
    concept Copyable
        = CopyConstructible<T> && Movable<T> && AssignableFrom<T&, T&>
       && AssignableFrom<T&, const T&> && AssignableFrom<T&, const T>;
    template <typename T>
    concept SemiRegular = Copyable<T> && DefaultInitializable<T>;
    template <typename T>
    concept Regular = SemiRegular<T> && std::equality_comparable<T>;

    namespace Ranges
    {
        namespace Swappable
        {
            template <typename T>
            void Swap(T&, T&) = delete;

            template <typename T, typename U>
            concept AdlSwap
                = (Details::ClassOrEnum<RemoveReferenceType<T>>
                   || Details::ClassOrEnum<RemoveReferenceType<U>>)
               && requires(T&& t, U&& u) {
                      Swap(static_cast<T &&>(t), static_cast<U &&>(u));
                  };

            struct SwapType
            {
              private:
                template <typename T, typename U>
                static constexpr bool NoExcept()
                {
                    if constexpr (AdlSwap<T, U>)
                        return PM_NOEXCEPT(Swap(DeclVal<T>(), DeclVal<U>()));
                    else
                        return IsNoThrowMoveConstructibleV<
                                   RemoveReferenceType<T>>
                            && IsNoThrowCopyConstructibleV<
                                   RemoveReferenceType<T>>;
                }

              public:
                template <typename T, typename U>
                    requires AdlSwap<T, U>
                          || (SameAs<T, U> && IsLValueReferenceV<T>
                              && MoveConstructible<RemoveReferenceType<T>>
                              && AssignableFrom<T, RemoveReferenceType<T>>)
                constexpr void operator()(T&& t, U&& u) const
                    PM_NOEXCEPT(NoExcept<T, U>())
                {
                    if constexpr (AdlSwap<T, U>)
                        Swap(static_cast<T&&>(t), static_cast<U&&>(u));
                    else
                    {
                        auto tmp = static_cast<RemoveReferenceType<T>&&>(t);
                        t        = static_cast<RemoveReferenceType<T>&&>(u);
                        u        = static_cast<RemoveReferenceType<T>&&>(tmp);
                    }
                }

                template <typename T, typename U, usize N>
                    requires requires(const SwapType& swap, T& e1, U& e2) {
                        swap(e1, e2);
                    }
                constexpr void operator()(T (&e1)[N], U (&e2)[N]) const
                    PM_NOEXCEPT(PM_NOEXCEPT(DeclVal<const SwapType&>()(*e1,
                                                                       *e2)))
                {
                    for (usize n = 0; n < N; ++n) (*this)(e1[n], e2[n]);
                }
            };
        }; // namespace Swappable

        inline namespace Cpo
        {
            inline constexpr Swappable::SwapType Swap{};
        }
    } // namespace Ranges

    template <typename T, typename U>
    concept SwappableWith
        = CommonReferenceWith<T, U> && requires(T&& t, U&& u) {
              Ranges::Swap(Forward<T>(t), Forward<T>(t));
              Ranges::Swap(Forward<U>(u), Forward<U>(u));
              Ranges::Swap(Forward<T>(t), Forward<U>(u));
              Ranges::Swap(Forward<U>(u), Forward<T>(t));
          };

    template <typename T>
    concept ArithmeticType = IsArithmeticV<T>;

    template <typename T>
    concept EnumType = IsEnumV<T>;
    template <typename T>
    concept IntegralOrEnum = IsIntegralV<T> || EnumType<T> || IsUnsignedV<T>;
    template <typename T>
    concept PrimitiveOrEnum
        = EnumType<T> || IsArithmeticV<T> || IsSameV<T, bool> || IsSignedV<T>
       || IsIntegralV<T>;

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

    template <typename T>
    concept ClassOrEnum = IsClassV<T> || IsUnionV<T> || IsEnumV<T>;
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::ArithmeticEnum;
using Prism::ConvertibleTo;
using Prism::DerivedFrom;
using Prism::EnumType;
using Prism::Integral;
using Prism::IntegralOrEnum;
using Prism::IsImplicitlyDefaultConstructible;
using Prism::PrimitiveOrEnum;
using Prism::SameAs;
using Prism::SignedIntegral;
using Prism::UnsignedIntegral;
#endif
