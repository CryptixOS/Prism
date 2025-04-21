/*
 * Created by v1tr10l7 on 06.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    namespace Details
    {
        template <typename T>
        inline constexpr bool IsIntegral = false;
        template <>
        inline constexpr bool IsIntegral<bool> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned char> = true;
        template <>
        inline constexpr bool IsIntegral<char8_t> = true;
        template <>
        inline constexpr bool IsIntegral<char16_t> = true;
        template <>
        inline constexpr bool IsIntegral<char32_t> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned short> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned int> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned long> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned long long> = true;

        template <bool Condition, typename TrueType, typename FalseType>
        struct Conditional
        {
            using Type = TrueType;
        };
        template <typename TrueType, typename FalseType>
        struct Conditional<false, TrueType, FalseType>
        {
            using Type = FalseType;
        };
    } // namespace Details

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
    template <typename T>
    inline constexpr bool IsEnum = __is_enum(T);

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
    struct RemoveVolatile<T volatile>
    {
        using Type = T;
    };
    template <typename T>
    using RemoveVolatileType = typename RemoveVolatile<T>::Type;

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
    using RemoveConstType = typename RemoveConst<T>::Type;
    template <typename T>
    using RemoveVolatileType = typename RemoveVolatile<T>::Type;
    template <typename T>
    using RemoveCvType = typename RemoveCv<T>::Type;

    template <typename T, typename U>
    struct IsSame : FalseType
    {
    };
    template <typename T>
    struct IsSame<T, T> : TrueType
    {
    };
    template <typename T, typename U>
    constexpr bool IsSameV = IsSame<T, U>::Value;

    template <typename T>
    struct IsVoid : IsSame<void, RemoveCvType<T>>
    {
    };
    template <typename T>
    constexpr bool IsVoidV = IsVoid<T>::Value;

    template <typename T>
    inline constexpr bool IsIntegral
        = Details::IsIntegral<std::make_unsigned<RemoveCvType<T>>>;

    template <bool Condition, typename TrueType, typename FalseType>
    using Conditional =
        typename Details::Conditional<Condition, TrueType, FalseType>::Type;
    template <typename T>
        requires(IsEnum<T>)
    using UnderlyingType = __underlying_type(T);

    template <typename T>
    struct PromoteEnum
    {
        using Type = Conditional<IsEnum<T>, UnderlyingType<T>, T>;
    };
    template <typename T>
    using PromoteEnumType = typename PromoteEnum<T>::Type;
    template <typename T>
    struct IsPointer : FalseType
    {
    };
    template <typename T>
    struct IsPointer<T*> : TrueType
    {
    };
    template <typename T>
    struct IsPointer<T* const> : TrueType
    {
    };
    template <typename T>
    struct IsPointer<T* volatile> : TrueType
    {
    };
    template <typename T>
    struct IsPointer<T* const volatile> : TrueType
    {
    };
    template <typename T>
    constexpr bool IsPointerV = IsPointer<T>::value;

    template <typename T, T... Ts>
    struct IntegerSequence
    {
        using Type = T;
        static constexpr usize Size() noexcept { return sizeof...(Ts); }
    };

    template <typename T, T N, T... Ts>
    auto MakeIntegerSequence()
    {
        if constexpr (N == 0) return IntegerSequence<T, Ts...>{};
        else return MakeIntegerSequence<T, N - 1, N - 1, Ts...>();
    }
    template <typename T>
    struct MakeUnsigned
    {
        using Type = void;
    };
    template <>
    struct MakeUnsigned<signed char>
    {
        using Type = unsigned char;
    };
    template <>
    struct MakeUnsigned<short>
    {
        using Type = unsigned short;
    };
    template <>
    struct MakeUnsigned<int>
    {
        using Type = unsigned int;
    };
    template <>
    struct MakeUnsigned<long>
    {
        using Type = unsigned long;
    };
    template <>
    struct MakeUnsigned<long long>
    {
        using Type = unsigned long long;
    };
    template <>
    struct MakeUnsigned<unsigned char>
    {
        using Type = unsigned char;
    };
    template <>
    struct MakeUnsigned<unsigned short>
    {
        using Type = unsigned short;
    };
    template <>
    struct MakeUnsigned<unsigned int>
    {
        using Type = unsigned int;
    };
    template <>
    struct MakeUnsigned<unsigned long>
    {
        using Type = unsigned long;
    };
    template <>
    struct MakeUnsigned<unsigned long long>
    {
        using Type = unsigned long long;
    };
    template <>
    struct MakeUnsigned<char>
    {
        using Type = unsigned char;
    };
    template <>
    struct MakeUnsigned<char8_t>
    {
        using Type = char8_t;
    };
    template <>
    struct MakeUnsigned<char16_t>
    {
        using Type = char16_t;
    };
    template <>
    struct MakeUnsigned<char32_t>
    {
        using Type = char32_t;
    };
    template <>
    struct MakeUnsigned<bool>
    {
        using Type = bool;
    };
    template <>
    struct MakeUnsigned<wchar_t>
    {
        using Type = wchar_t;
    };

    template <typename T>
    struct RemoveCvReference;

    template <typename T>
    struct Decay
    {
        using Type = RemoveCvReference<T>;
    };
    template <typename T>
    struct Decay<T[]>
    {
        using Type = T*;
    };
    template <typename T, decltype(sizeof(T)) N>
    struct Decay<T[N]>
    {
        using Type = T*;
    };

    template <usize... Indices>
    using IndexSequence = IntegerSequence<usize, Indices...>;
    template <typename T, T N>
    using MakeIntegerSequenceV = decltype(MakeIntegerSequence<T, N>());
    template <usize N>
    using MakeIndexSequenceV = MakeIntegerSequenceV<usize, N>;
}; // namespace Prism
