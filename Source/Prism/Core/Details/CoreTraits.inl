/*
 * Created by v1tr10l7 on 01.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T, T V>
    struct IntegralConstant
    {
        static constexpr T Value = V;
        using ValueType          = T;
        using Type               = IntegralConstant;

        constexpr           operator ValueType() const { return Value; }
        constexpr ValueType operator()() const { return Value; }
    };
    using TrueType  = IntegralConstant<bool, true>;
    using FalseType = IntegralConstant<bool, false>;

    template <bool Boolean>
    using BooleanConstant = IntegralConstant<bool, Boolean>;

    template <typename...>
    using VoidType = void;
    template <typename T>
    struct TypeIdentity
    {
        using Type = T;
    };
    template <typename T>
    using TypeIdentityType = TypeIdentity<T>::Type;

    template <typename T, typename = void>
    struct AddRValueReference;

    template <typename T>
    typename AddRValueReference<T>::Type DeclVal() PM_NOEXCEPT
    {
        static_assert(false, "declval not allowed in an evaluated context");
    }
    template <typename T>
    auto declval() -> T;

    template <typename T, usize = sizeof(T)>
    constexpr TrueType IsCompleteOrUnbounded(TypeIdentity<T>)
    {
        return {};
    }
    template <typename T>
    struct IsArrayUnknownBounds : public FalseType
    {
    };
    template <typename T>
    struct IsArrayUnknownBounds<T[]> : public TrueType
    {
    };

    template <typename T>
    struct IsReference;
    template <typename T>
    struct IsFunction;
    template <typename T>
    struct IsVoid;
    template <typename... Args>
    struct Disjunction;
    template <typename TypeId, typename NestedType = typename TypeId::Type>
    constexpr Disjunction<IsReference<NestedType>, IsFunction<NestedType>,
                          IsVoid<NestedType>,
                          IsArrayUnknownBounds<NestedType>>::Type
    IsCompleteOrUnbounded(TypeId)
    {
        return {};
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::DeclVal;
#endif
