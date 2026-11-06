/*
 * Created by v1tr10l7 on 01.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/TransformationTraits.inl>

namespace Prism
{
    template <typename T, typename... Args>
    using IsConstructibleHelper
        = BooleanConstant<__is_constructible(T, Args...)>;
    template <typename T, typename... Args>
    struct IsConstructible : public IsConstructibleHelper<T, Args...>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };
    template <typename T, typename... Args>
    using IsTriviallyConstructibleHelper
        = BooleanConstant<__is_trivially_constructible(T, Args...)>;
    template <typename T, typename... Args>
    struct IsTriviallyConstructible
        : public IsTriviallyConstructibleHelper<T, Args...>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };
    template <typename T, typename... Args>
    using IsNoThrowConstructibleHelper
        = BooleanConstant<__is_nothrow_constructible(T, Args...)>;
    template <typename T, typename... Args>
    struct IsNoThrowConstructible
        : public IsNoThrowConstructibleHelper<T, Args...>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };

    template <typename T>
    struct IsDefaultConstructible : public IsConstructibleHelper<T>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };
    template <typename T>
    struct IsTriviallyDefaultConstructible
        : public IsTriviallyConstructibleHelper<T>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };
    template <typename T>
    struct IsNoThrowDefaultConstructible : public IsNoThrowConstructible<T>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };

    template <typename T>
    struct IsCopyConstructible
        : public IsConstructibleHelper<T, AddLValueReferenceType<const T>>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };
    template <typename T>
    struct IsTriviallyCopyConstructible
        : public IsTriviallyConstructibleHelper<T,
                                                AddLValueReferenceType<const T>>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };
    template <typename T>
    struct IsNoThrowCopyConstructible
        : public IsNoThrowConstructibleHelper<T,
                                              AddLValueReferenceType<const T>>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };

    template <typename T>
    struct IsMoveConstructible
        : public IsConstructibleHelper<T, AddRValueReferenceType<T>>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };

    template <typename T>
    struct IsTriviallyMoveConstructible
        : public IsTriviallyConstructibleHelper<T, AddRValueReferenceType<T>>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };
    template <typename T>
    struct IsNoThrowMoveConstructible
        : public IsNoThrowConstructibleHelper<T, AddRValueReferenceType<T>>
    {
        // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //               "template argument must be a complete class or an "
        //               "unbounded array!");
    };

    template <typename T, typename U>
    using IsAssignableHelper = BooleanConstant<__is_assignable(T, U)>;
    template <typename T, typename U>
    struct IsAssignable : public IsAssignableHelper<T, U>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    template <typename T, typename U>
    using IsTriviallyAssignableHelper
        = BooleanConstant<__is_trivially_assignable(T, U)>;
    template <typename T, typename U>
    struct IsTriviallyAssignable : public IsTriviallyAssignableHelper<T, U>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };

    template <typename T, typename U>
    using IsNoThrowAssignableHelper
        = BooleanConstant<__is_nothrow_assignable(T, U)>;
    template <typename T, typename U>
    struct IsNoThrowAssignable : public IsNoThrowAssignableHelper<T, U>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    template <typename T>
    struct IsCopyAssignable
        : IsAssignable<AddLValueReferenceType<T>,
                       AddLValueReferenceType<const T>>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };

    template <typename T>
    struct IsTriviallyCopyAssignable
        : IsTriviallyAssignable<AddLValueReferenceType<T>,
                                AddLValueReferenceType<const T>>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };

    template <typename T>
    struct IsNoThrowCopyAssignable
        : IsNoThrowAssignable<AddLValueReferenceType<T>,
                              AddLValueReferenceType<const T>>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    template <typename T>
    using IsMoveAssignableHelper
        = BooleanConstant<__is_assignable(AddLValueReferenceType<T>,
                                          AddRValueReferenceType<T>)>;
    template <typename T>
    struct IsMoveAssignable : IsMoveAssignableHelper<T>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    template <typename T>
    using IsTriviallyMoveAssignableHelper
        = BooleanConstant<__is_trivially_assignable(
            AddLValueReferenceType<T>, AddRValueReferenceType<T>)>;

    template <typename T>
    struct IsTriviallyMoveAssignable : IsTriviallyMoveAssignableHelper<T>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    template <typename T>
    using IsNoThrowMoveAssignableHelper
        = BooleanConstant<__is_nothrow_assignable(AddLValueReferenceType<T>,
                                                  AddRValueReferenceType<T>)>;
    template <typename T>
    struct IsNoThrowMoveAssignable : IsNoThrowMoveAssignableHelper<T>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    namespace Details
    {
        // std::is_destructible semantics: references are destructible,
        // arrays of known bound are destructible if their element is,
        // void, functions and arrays of unknown bound are not.
        template <typename T>
        struct DestructibleProbe
        {
            static constexpr bool Value = requires(T& t) { t.~T(); };
            static constexpr bool NoThrow
                = requires(T& t) { { t.~T() } noexcept; };
        };
        template <typename T>
        struct DestructibleProbe<T&>
        {
            static constexpr bool Value   = true;
            static constexpr bool NoThrow = true;
        };
        template <typename T>
        struct DestructibleProbe<T&&>
        {
            static constexpr bool Value   = true;
            static constexpr bool NoThrow = true;
        };
        template <typename T, usize N>
        struct DestructibleProbe<T[N]> : DestructibleProbe<T>
        {
        };
        template <typename T>
        struct DestructibleProbe<T[]>
        {
            static constexpr bool Value   = false;
            static constexpr bool NoThrow = false;
        };
    } // namespace Details

    template <typename T>
    using IsDestructibleHelper
#if PrismHasBuiltin(__is_destructible)
        = BooleanConstant<__is_destructible(T)>;
#else
        = BooleanConstant<Details::DestructibleProbe<T>::Value>;
#endif
    template <typename T>
    struct IsDestructible : IsDestructibleHelper<T>
    {
    };
    template <typename T>
    using IsNoThrowDestructibleHelper
#if PrismHasBuiltin(__is_nothrow_destructible)
        = BooleanConstant<__is_nothrow_destructible(T)>;
#else
        = BooleanConstant<Details::DestructibleProbe<T>::NoThrow>;
#endif
    template <typename T>
    struct IsNoThrowDestructible : IsNoThrowDestructibleHelper<T>
    {
    };
    template <typename T>
    struct HasVirtualDestructor : BooleanConstant<__has_virtual_destructor(T)>
    {
    };
    template <typename T>
    using IsTriviallyDestructibleHelper
#if PrismHasBuiltin(__is_trivially_destructible)
        = BooleanConstant<__is_trivially_destructible(T)>;
#else
        = BooleanConstant<__has_trivial_destructor(T)
                          && IsDestructible<T>::Value>;
#endif

    template <typename T>
    struct IsTriviallyDestructible : IsTriviallyDestructibleHelper<T>
    {
    };
    // Swappability
    //
    // Prism::Swap(T&, T&) is declared in Core.hpp, after these traits, so it
    // can't be found by ordinary lookup here (and never by ADL for
    // fundamental types). Mirror its contract instead: a user-provided Swap
    // found by ADL wins, otherwise a type is swappable when it is move
    // constructible and move assignable; arrays are swappable when their
    // elements are.
    namespace Details::SwapAdl
    {
        // Poison pills: stop unqualified lookup from leaving this namespace.
        template <typename T>
        void Swap(T&, T&) = delete;
        template <typename T, usize N>
        void Swap(T (&)[N], T (&)[N]) = delete;

        template <typename T>
        concept ClassOrEnumLike = __is_class(RemoveReferenceType<T>)
                               || __is_union(RemoveReferenceType<T>)
                               || __is_enum(RemoveReferenceType<T>);

        template <typename T, typename U>
        concept AdlSwappable
            = (ClassOrEnumLike<T> || ClassOrEnumLike<U>)
           && requires(T&& t, U&& u) {
                  Swap(static_cast<T&&>(t), static_cast<U&&>(u));
              };
        template <typename T, typename U>
        concept AdlNoThrowSwappable
            = AdlSwappable<T, U> && requires(T&& t, U&& u) {
                  { Swap(static_cast<T&&>(t), static_cast<U&&>(u)) } noexcept;
              };

        template <typename T, typename U>
        struct SwapProbe
        {
            static constexpr bool Value   = AdlSwappable<T, U>;
            static constexpr bool NoThrow = AdlNoThrowSwappable<T, U>;
        };
        template <typename T>
        struct SwapProbe<T&, T&>
        {
            static constexpr bool Value
                = AdlSwappable<T&, T&>
               || (IsMoveConstructible<T>::Value && IsMoveAssignable<T>::Value);
            static constexpr bool NoThrow
                = AdlSwappable<T&, T&>
                    ? AdlNoThrowSwappable<T&, T&>
                    : (IsNoThrowMoveConstructible<T>::Value
                       && IsNoThrowMoveAssignable<T>::Value);
        };
        template <typename T, usize N>
        struct SwapProbe<T (&)[N], T (&)[N]>
        {
            static constexpr bool Value   = SwapProbe<T&, T&>::Value;
            static constexpr bool NoThrow = SwapProbe<T&, T&>::NoThrow;
        };
    } // namespace Details::SwapAdl

    template <typename T, typename U>
    struct IsSwappableWith
        : BooleanConstant<Details::SwapAdl::SwapProbe<T, U>::Value
                          && Details::SwapAdl::SwapProbe<U, T>::Value>
    {
    };
    template <typename T>
    struct IsSwappable
        : IsSwappableWith<AddLValueReferenceType<T>, AddLValueReferenceType<T>>
    {
    };

    template <typename T, typename U>
    struct IsNoThrowSwappableWith
        : BooleanConstant<IsSwappableWith<T, U>::Value
                          && Details::SwapAdl::SwapProbe<T, U>::NoThrow
                          && Details::SwapAdl::SwapProbe<U, T>::NoThrow>
    {
    };
    template <typename T>
    struct IsNoThrowSwappable
        : IsNoThrowSwappableWith<AddLValueReferenceType<T>,
                                 AddLValueReferenceType<T>>
    {
    };
#if __has_builtin(__reference_constructs_from_temporary)
    template <typename T, typename U>
    struct ReferenceConstructsFromTemporary
        : BooleanConstant<__reference_constructs_from_temporary(T, U)>
    {
    };
#endif
#if __has_builtin(__reference_converts_from_temporary)
    template <typename T, typename U>
    struct ReferenceConvertsFromTemporary
        : BooleanConstant<__reference_converts_from_temporary(T, U)>
    {
    };
#endif

    template <typename T, typename... Args>
    inline constexpr bool IsConstructibleV = IsConstructible<T, Args...>::Value;
    template <typename T, typename... Args>
    inline constexpr bool IsTriviallyConstructibleV
        = IsTriviallyConstructible<T, Args...>::Value;
    template <typename T, typename... Args>
    inline constexpr bool IsNoThrowConstructibleV
        = IsNoThrowConstructible<T, Args...>::Value;

    template <typename T>
    inline constexpr bool IsDefaultConstructibleV
        = IsDefaultConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyDefaultConstructibleV
        = IsTriviallyDefaultConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowDefaultConstructibleV
        = IsNoThrowDefaultConstructible<T>::Value;

    template <typename T>
    inline constexpr bool IsCopyConstructibleV = IsCopyConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyCopyConstructibleV
        = IsTriviallyCopyConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowCopyConstructibleV
        = IsNoThrowCopyConstructible<T>::Value;

    template <typename T>
    inline constexpr bool IsMoveConstructibleV = IsMoveConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyMoveConstructibleV
        = IsTriviallyMoveConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowMoveConstructibleV
        = IsNoThrowMoveConstructible<T>::Value;

    template <typename T, typename U>
    inline constexpr bool IsAssignableV = IsAssignable<T, U>::Value;
    template <typename T, typename U>
    inline constexpr bool IsTriviallyAssignableV
        = IsTriviallyAssignable<T, U>::Value;
    template <typename T, typename U>
    inline constexpr bool IsNoThrowAssignableV
        = IsNoThrowAssignable<T, U>::Value;

    template <typename T>
    inline constexpr bool IsCopyAssignableV = IsCopyAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyCopyAssignableV
        = IsTriviallyCopyAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowCopyAssignableV
        = IsNoThrowCopyAssignable<T>::Value;

    template <typename T>
    inline constexpr bool IsMoveAssignableV = IsMoveAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyMoveAssignableV
        = IsTriviallyMoveAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowMoveAssignableV
        = IsNoThrowMoveAssignable<T>::Value;

    template <typename T>
    inline constexpr bool IsDestructibleV = IsDestructible<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyDestructibleV
        = IsTriviallyDestructible<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowDestructibleV
        = IsNoThrowDestructible<T>::Value;
    template <typename T>
    inline constexpr bool HasVirtualDestructorV
        = HasVirtualDestructor<T>::Value;

    template <typename T, typename U>
    inline constexpr bool IsSwappableWithV = IsSwappableWith<T, U>::Value;
    template <typename T>
    inline constexpr bool IsSwappableV = IsSwappable<T>::Value;
    template <typename T, typename U>
    inline constexpr bool IsNoThrowSwappableWithV
        = IsNoThrowSwappableWith<T, U>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowSwappableV = IsNoThrowSwappable<T>::Value;

    template <typename To, typename From>
    using IsArrayConvertible = IsConvertible<From (*)[], To (*)[]>;
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::IsTriviallyDestructible;
using Prism::IsTriviallyDestructibleV;
#endif
