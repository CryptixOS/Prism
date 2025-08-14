/*
 * Created by v1tr10l7 on 02.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/CVRefTraits.inl>

namespace Prism
{
    template <bool B, typename T, typename F>
    struct Conditional
    {
        using Type = T;
    };
    template <typename T, typename F>
    struct Conditional<false, T, F>
    {
        using Type = F;
    };
    template <bool B, typename T, typename F>
    using ConditionalType = typename Conditional<B, T, F>::Type;

    template <bool B, typename T = void>
    struct EnableIf
    {
    };
    template <typename T>
    struct EnableIf<true, T>
    {
        using Type = T;
    };
    template <bool B, typename T = void>
    using EnableIfType = typename EnableIf<B, T>::Type;

    template <typename T, typename U>
    struct IsSame : FalseType
    {
    };
    template <typename T>
    struct IsSame<T, T> : TrueType
    {
    };
    template <typename T, typename U>
    inline constexpr bool IsSameV = IsSame<T, U>::Value;

    template <typename... Args>
    struct Conjunction : TrueType
    {
    };
    template <typename B1>
    struct Conjunction<B1> : B1
    {
    };
    template <typename B1, typename... Bn>
    struct Conjunction<B1, Bn...>
        : ConditionalType<bool(B1::Value), Conjunction<Bn...>, B1>
    {
    };
    template <typename... Args>
    struct Disjunction : FalseType
    {
    };
    template <typename B1>
    struct Disjunction<B1> : B1
    {
    };
    template <typename B1, typename... Bn>
    struct Disjunction<B1, Bn...>
        : ConditionalType<bool(B1::Value), B1, Disjunction<Bn...>>
    {
    };
    template <typename B>
    struct Negation : BooleanConstant<!bool(B::Value)>
    {
    };
    template <typename T>
    constexpr RemoveReferenceType<T>&& Move(T&& object) PM_NOEXCEPT
    {
        return static_cast<RemoveReferenceType<T>&&>(object);
    }

    // FIXME(v1tr10l7): Why is it here?
    template <typename T>
    constexpr T&& Forward(RemoveReferenceType<T>& value) PM_NOEXCEPT
    {
        return static_cast<T&&>(value);
    }
    template <typename T>
    constexpr T&& Forward(RemoveReferenceType<T>&& value) PM_NOEXCEPT
    {
        static_assert(!IsLValueReferenceV<T>,
                      "Don't forward an rvalue as an lvalue");
        return static_cast<T&&>(value);
    }
    template <typename... Args>
    inline constexpr bool ConjunctionV = Conjunction<Args...>::Value;
    template <typename... Args>
    inline constexpr bool DisjunctionV = Disjunction<Args...>::Value;
    template <typename T>
    inline constexpr bool NegationV = Negation<T>::Value;
}; // namespace Prism
