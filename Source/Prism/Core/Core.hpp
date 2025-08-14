/*
 * Created by v1tr10l7 on 03.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/TypeTraits.hpp>

namespace Prism
{
    template <typename T>
    [[nodiscard]]
    inline constexpr T* AddressOf(T& object) PM_NOEXCEPT
    {
        return __builtin_addressof(object);
    }

    template <typename T>
    const T* AddressOf(const T&&) = delete;

    template <typename T>
    constexpr inline EnableIfType<
        Conjunction<Negation<IsTupleLike<T>>, IsMoveConstructible<T>,
                    IsMoveAssignable<T>>::Value>::Type
    Swap(T& lhs, T& rhs)
        PM_NOEXCEPT(Conjunction<IsNoThrowMoveConstructible<T>,
                                IsNoThrowMoveAssignable<T>>::Value)
    {
        if (&lhs == &rhs) return;

        T temp = Move(lhs);
        lhs    = Move(rhs);
        rhs    = Move(temp);
    }

    template <typename T, usize Count>
    constexpr inline EnableIfType<IsSwappable<T>::Value>::Type
    Swap(T (&lhs)[Count], T (&rhs)[Count])
        PM_NOEXCEPT(IsNoThrowSwappable<T>::Value)
    {
        for (usize i = 0; i < Count; ++i) Swap(lhs[i], rhs[i]);
    }

    template <typename T, typename U = T>
    constexpr inline T Exchange(T& object, U&& newValue) PM_NOEXCEPT(
        IsNoThrowMoveConstructibleV<T>&& IsNoThrowAssignableV<T&, U>)
    {
        T oldValue = Move(object);
        object     = Forward<U>(newValue);
        return oldValue;
    }
    template <typename T, typename U,
              typename
              = EnableIf<(IsAssignableV<T&, U> && IsMoveConstructibleV<T>)>>
    constexpr inline void Swap(T& lhs, U& rhs)
    {
        if (&lhs == &rhs) return;
        lhs = Exchange(rhs, lhs);
    }

    template <typename T>
    inline constexpr AddConstType<T>& AsConst(T& t) PM_NOEXCEPT
    {
        return t;
    }
    template <typename T>
    void AsConst(const T&&) = delete;
    template <typename T, typename U>
    constexpr auto&& ForwardLike(U&& x) PM_NOEXCEPT
    {
        constexpr bool isAddingConst = IsConstV<RemoveReferenceType<T>>;
        if constexpr (IsLValueReferenceV<T&&>)
        {
            if constexpr (isAddingConst) return AsConst(x);
            else return static_cast<U&>(x);
        }
        else
        {
            if constexpr (isAddingConst) return Move(AsConst(x));
            else return Move(x);
        }
    }

    [[noreturn]]
    PM_ALWAYS_INLINE void Unreachable()
    {
#ifdef PM_DEBUG
        PrismAssertionFailed("Unreachable()");
#elif defined PM_ENABLE_ASSERTIONS
        __builtin_trap();
#else
        __builtin_unreachable();
#endif
    }

    template <typename T, T... Ts>
    struct IntegerSequence
    {
        static_assert(IsIntegralV<T>);

        using ValueType = T;
        static constexpr usize Size() PM_NOEXCEPT { return sizeof...(Ts); }
    };
    template <typename T, T Count>
    using MakeIntegerSequence
#if PrismHasBuiltin(__make_integer_seq)
        = __make_integer_seq<IntegerSequence, T, Count>;
#else
        = IntegerSequence<T, __integer_pack(Count)...>;
#endif
    template <usize... Indices>
    using IndexSequence = IntegerSequence<usize, Indices...>;
    template <usize Count>
    using MakeIndexSequence = MakeIntegerSequence<usize, Count>;

    template <typename... Types>
    using IndexSequenceFor = MakeIndexSequence<sizeof...(Types)>;

    struct InPlaceT
    {
        explicit InPlaceT() = default;
    };
    inline constexpr InPlaceT InPlace{};
    template <typename T>
    struct InPlaceTypeT
    {
        explicit InPlaceTypeT() = default;
    };
    template <typename T>
    constexpr InPlaceTypeT<T> InPlaceType{};
    template <usize I>
    struct InPlaceIndexT
    {
        explicit InPlaceIndexT() = default;
    };
    template <usize I>
    constexpr InPlaceIndexT<I> InPlaceIndex{};

    template <typename T, typename... Args>
    constexpr auto ConstructAt(T* location, Args&&... args)
        PM_NOEXCEPT(PM_NOEXCEPT(::new((void*)0) T(DeclVal<Args>()...)))
            -> decltype(::new((void*)0) T(DeclVal<Args>()...))
    {
        assert(location);
        return ::new (reinterpret_cast<void*>(location))
            T(Forward<Args>(args)...);
    }
    template <typename T>
    constexpr void DestroyAt(T* object)
    {
        if constexpr (IsArrayV<T>)
            for (auto& value : *object) DestoryAt(AddressOf(value));
        else object->~T();
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::AddConstType;
using Prism::AddressOf;
using Prism::AsConst;
using Prism::ConstructAt;
using Prism::DestroyAt;
using Prism::Exchange;
using Prism::IndexSequence;
using Prism::IndexSequenceFor;
using Prism::InPlaceT;
using Prism::IntegerSequence;
using Prism::MakeIndexSequence;
using Prism::MakeIntegerSequence;
using Prism::Swap;
using Prism::Unreachable;
#endif
