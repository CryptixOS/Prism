/*
 * Created by v1tr10l7 on 06.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Details/CallableTraits.inl>
#include <Prism/Core/Details/Constructibility.inl>
#include <Prism/Core/Details/ExtendedTraits.inl>

namespace Prism
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
#if PrismHasBuiltin(__builtin_is_within_lifetime)
    /// `ptr` points to an object that is within its lifetime (C++26).
    template <typename T>
    consteval bool IsWithinLifetime(const T* ptr) PM_NOEXCEPT
    {
        return __builtin_is_within_lifetime(ptr);
    }
#endif

    // Member relationships
#if PrismHasBuiltin(__builtin_is_pointer_interconvertible_with_class)
    /// Objects of type S are pointer-interconvertible with their member
    /// `member` (std::is_pointer_interconvertible_with_class).
    template <typename S, typename M>
    constexpr bool IsPointerInterconvertibleWithClass(M S::* member) PM_NOEXCEPT
    {
        return __builtin_is_pointer_interconvertible_with_class(member);
    }
#endif
#if PrismHasBuiltin(__builtin_is_corresponding_member)
    /// `m1` and `m2` refer to corresponding members in the common initial
    /// sequence of S1 and S2 (std::is_corresponding_member).
    template <typename S1, typename S2, typename M1, typename M2>
    constexpr bool IsCorrespondingMember(M1 S1::* m1,
                                         M2 S2::* m2) PM_NOEXCEPT
    {
        return __builtin_is_corresponding_member(m1, m2);
    }
#endif

    template <typename T>
    constexpr bool IsTupleLikeV = IsTupleLike<T>::Value;
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::Conditional;
using Prism::Forward;
using Prism::IsSameV;
using Prism::IsTupleLikeV;
using Prism::Move;
#endif
