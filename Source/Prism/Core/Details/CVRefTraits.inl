/*
 * Created by v1tr10l7 on 01.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/CoreTraits.inl>

namespace Prism
{
    template <typename T>
    struct RemoveConst
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveConst<const T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveVolatile
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveVolatile<volatile T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveCv
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveCv<const T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveCv<volatile T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveCv<const volatile T>
    {
        using Type = T;
    };

    template <typename T>
    struct AddConst
    {
        using Type = const T;
    };
    template <typename T>
    struct AddVolatile
    {
        using Type = volatile T;
    };
    template <typename T>
    struct AddCv
    {
        using Type = const volatile T;
    };

    template <typename T>
    using RemoveConstType = typename RemoveConst<T>::Type;
    template <typename T>
    using RemoveVolatileType = typename RemoveVolatile<T>::Type;
    template <typename T>
    using RemoveCvType = typename RemoveCv<T>::Type;

    template <typename T>
    using AddConstType = typename AddConst<T>::Type;
    template <typename T>
    using AddVolatileType = typename AddVolatile<T>::Type;
    template <typename T>
    using AddCvType = typename AddCv<T>::Type;

    template <typename T>
    struct RemoveReference
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveReference<T&>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveReference<T&&>
    {
        using Type = T;
    };
    template <typename T, typename = void>
    struct AddLValueReference
    {
        using Type = T;
    };
    template <typename T>
    struct AddLValueReference<T, VoidType<T&>>
    {
        using Type = T&;
    };
    template <typename T, typename>
    struct AddRValueReference
    {
        using Type = T;
    };
    template <typename T>
    struct AddRValueReference<T, VoidType<T&&>>
    {
        using Type = T&&;
    };

    template <typename T>
    using RemoveReferenceType = typename RemoveReference<T>::Type;
    template <typename T>
    using AddLValueReferenceType = typename AddLValueReference<T>::Type;
    template <typename T>
    using AddRValueReferenceType = typename AddRValueReference<T>::Type;

    template <typename T>
    struct RemoveCvRef
    {
        using Type = RemoveCvType<RemoveReferenceType<T>>;
    };
    template <typename T>
    using RemoveCvRefType = RemoveCvRef<T>::Type;

    template <typename T>
    struct IsLValueReference : FalseType
    {
    };
    template <typename T>
    struct IsLValueReference<T&> : TrueType
    {
    };
    template <typename T>
    struct IsRValueReference : FalseType
    {
    };
    template <typename T>
    struct IsRValueReference<T&&> : TrueType
    {
    };
    template <typename T>
    struct IsReference : FalseType
    {
    };
    template <typename T>
    struct IsReference<T&> : TrueType
    {
    };
    template <typename T>
    struct IsReference<T&&> : TrueType
    {
    };

    template <typename T>
    inline constexpr bool IsLValueReferenceV = IsLValueReference<T>::Value;
    template <typename T>
    inline constexpr bool IsRValueReferenceV = IsRValueReference<T>::Value;
    template <typename T>
    inline constexpr bool IsReferenceV = IsReference<T>::Value;
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::RemoveCvRefType;
using Prism::RemoveReference;
using Prism::RemoveReferenceType;
#endif
