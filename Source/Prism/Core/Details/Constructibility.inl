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
    struct IsCopyAssignable : IsTriviallyAssignable<T&, const T&>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };

    template <typename T>
    struct IsTriviallyCopyAssignable : IsTriviallyAssignable<T&, const T&>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };

    template <typename T>
    struct IsNoThrowCopyAssignable : IsNoThrowAssignable<T&, const T&>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    template <typename T>
    using IsMoveAssignableHelper = BooleanConstant<__is_assignable(T&, T&&)>;
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
        = BooleanConstant<__is_trivially_assignable(T&, T&&)>;

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
        = BooleanConstant<__is_nothrow_assignable(T&, T&&)>;
    template <typename T>
    struct IsNoThrowMoveAssignable : IsNoThrowMoveAssignableHelper<T>
    {
        // static_assert(
        //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
        //     "template argument must be a complete class or an unbounded
        //     array");
    };
    template <typename T>
    using IsDestructibleHelper
        = BooleanConstant<requires { DeclVal<T>().~T(); }>;
    template <typename T>
    struct IsDestructible : IsDestructibleHelper<T>
    {
    };
    // template <typename T>
    // using IsTriviallyDestructibleHelper
    // = BooleanConstant<__is_trivially_destructible(T)>;
    // template <typename T>
    // struct IsTriviallyDestructible : IsTriviallyDestructibleHelper<T>
    // {
    // };
    // template <typename T>
    // using IsNoThrowDestructibleHelper
    //     = BooleanConstant<__is_nothrow_destructible(T)>;
    // template <typename T>
    // struct IsNoThrowDestructible : IsNoThrowDestructibleHelper<T>
    // {
    // };
    // template <typename T>
    // struct HasVirtualDestructor :
    // BooleanConstant<__has_virtual_destructor(T)>
    // {
    // };
    // Swappability (C++17-style detection)
    namespace Detail
    {
        using std::swap;

        template <typename T, typename U>
        constexpr auto TestSwap(int)
            -> decltype(swap(DeclVal<T>(), DeclVal<U>()), TrueType{});

        template <typename, typename>
        constexpr FalseType TestSwap(...);

        template <typename T, typename U>
        constexpr auto TestNoThrowSwap(int)
            -> BooleanConstant<noexcept(swap(DeclVal<T>(), DeclVal<U>()))>;
        template <typename, typename>
        constexpr FalseType TestNoThrowSwap(...);
    } // namespace Detail

    template <typename T, typename U>
    struct IsSwappableWith : decltype(Detail::TestSwap<T, U>(0))
    {
    };
    template <typename T>
    struct IsSwappable : IsSwappableWith<T&, T&>
    {
    };
    template <typename T, typename U>
    struct IsNoThrowSwappableWith : decltype(Detail::TestNoThrowSwap<T, U>(0))
    {
    };
    template <typename T>
    struct IsNoThrowSwappable : IsNoThrowSwappableWith<T&, T&>
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
    // template <typename T>
    // inline constexpr bool IsTriviallyDestructibleV
    //     = IsTriviallyDestructible<T>::Value;
    // template <typename T>
    // inline constexpr bool IsNoThrowDestructibleV
    //     = IsNoThrowDestructible<T>::Value;

    // template <typename T>
    // inline constexpr bool HasVirtualDestructorV
    //     = HasVirtualDestructor<T>::Value;

    template <typename T, typename U>
    inline constexpr bool IsSwappableWithV = IsSwappableWith<T, U>::Value;
    template <typename T>
    inline constexpr bool IsSwappableV = IsSwappable<T>::Value;
    template <typename T, typename U>
    inline constexpr bool IsNoThrowSwappableWithV
        = IsNoThrowSwappableWith<T, U>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowSwappableV = IsNoThrowSwappable<T>::Value;
}; // namespace Prism
