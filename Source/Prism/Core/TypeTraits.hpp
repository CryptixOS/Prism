/*
 * Created by v1tr10l7 on 06.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/CallableTraits.inl>
#include <Prism/Core/Details/Constructibility.inl>

PRISM_EXPORT namespace Prism
{
    template <typename T, bool>
    struct EnumUnderlying
    {
    };
    template <typename T>
    struct EnumUnderlying<T, true>
    {
        using Type = UnderlyingType<T>;
    };
    template <typename T>
    struct EnumUnderlying<T, false>
    {
        using Type = T;
    };

    template <typename>
    struct IsTupleLikeHelper : FalseType
    {
    };

    template <typename T>
    struct IsTupleLike : public IsTupleLikeHelper<RemoveCvRefType<T>>::Type
    {
    };

    template <typename T>
    struct PromoteEnum
    {
        using Type
            = ConditionalType<IsEnumV<T>,
                              typename EnumUnderlying<T, IsEnumV<T>>::Type, T>;
    };
    template <typename T>
    using PromoteEnumType = typename PromoteEnum<T>::Type;

    constexpr bool IsConstantEvaluated() PM_NOEXCEPT
    {
        if consteval { return true; }
        return false;
    }
    // template <typename T>
    // consteval bool IsWithinLifetime(const T* ptr) PM_NOEXCEPT
    // {
    // }

    template <typename T>
    inline constexpr bool IsTupleLikeV = IsTupleLike<T>::Value;
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
PRISM_EXPORT using Prism::Conditional;
PRISM_EXPORT using Prism::Forward;
PRISM_EXPORT using Prism::IsSameV;
PRISM_EXPORT using Prism::IsTupleLikeV;
PRISM_EXPORT using Prism::Move;
#endif
