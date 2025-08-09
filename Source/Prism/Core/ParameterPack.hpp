/*
 * Created by v1tr10l7 on 09.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Core.hpp>

namespace Prism
{
    template <typename... Types>
    struct ParameterPack;

    template <unsigned Index, typename List>
    struct ParameterPackElement;

#if PrismHasBuiltin(__type_pack_element)
    template <u32 Index, typename... Types>
    struct ParameterPackElement<Index, ParameterPack<Types...>>
    {
        using Type = __type_pack_element<Index, Types...>;
    };
#else
    template <i32 Index, typename Head, typename... Tail>
    struct ParameterPackElement<Index, ParameterPack<Head, Tail...>>
        : ParameterPackElement<Index - 1, ParameterPack<Tail...>>
    {
    };

    template <typename Head, typename... Tail>
    struct ParameterPackElement<0, ParameterPack<Head, Tail...>>
    {
        using Type = Head;
    };
#endif

    template <typename... Types>
    struct ParameterPack
    {
        static constexpr unsigned Size = sizeof...(Types);

        template <unsigned N>
        using Type =
            typename ParameterPackElement<N, ParameterPack<Types...>>::Type;
    };

    template <typename T>
    struct TypeWrapper
    {
        using Type = T;
    };

    template <typename List, typename F, usize... Indices>
    constexpr void ForEachTypeImpl(F&& f, IndexSequence<Indices...>)
    {
        (Forward<F>(f)(TypeWrapper<typename List::template Type<Indices>>{}),
         ...);
    }

    template <typename List, typename F>
    constexpr void ForEachType(F&& f)
    {
        ForEachTypeImpl<List>(Forward<F>(f), MakeIndexSequence<List::Size>{});
    }

    template <typename ListA, typename ListB, typename F, usize... Indices>
    constexpr void ForEachTypeZippedImpl(F&& f, IndexSequence<Indices...>)
    {
        (Forward<F>(f)(TypeWrapper<typename ListA::template Type<Indices>>{},
                       TypeWrapper<typename ListB::template Type<Indices>>{}),
         ...);
    }

    template <typename ListA, typename ListB, typename F>
    constexpr void ForEachTypeZipped(F&& f)
    {
        static_assert(ListA::Size == ListB::Size,
                      "Can't zip ParameterPacks that aren't the same size!");
        ForEachTypeZippedImpl<ListA, ListB>(Forward<F>(f),
                                            MakeIndexSequence<ListA::Size>{});
    }

}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::ParameterPack;
#endif
