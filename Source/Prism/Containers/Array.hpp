/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Core.hpp>
#include <Prism/Core/Iterator.hpp>

#include <Prism/Debug/Assertions.hpp>
#include <Prism/Memory/Memory.hpp>

namespace Prism
{
    template <typename T, usize N>
    struct ArrayTraits
    {
        using Type               = T[N];
        using IsSwappable        = Details::IsSwappable<T>;
        using IsNoThrowSwappable = Details::IsNoThrowSwappable<T>;
    };

    template <typename T>
    struct ArrayTraits<T, 0>
    {
        struct Type
        {
            PM_NORETURN PM_ALWAYS_INLINE T& operator[](usize) const PM_NOEXCEPT
            {
                __builtin_unreachable();
            }

            PM_ALWAYS_INLINE constexpr explicit operator T*() const PM_NOEXCEPT
            {
                return nullptr;
            }
        };

        using IsSwappable        = TrueType;
        using IsNoThrowSwappable = TrueType;
    };

    template <typename T, usize N>
    struct Array
    {
        using ValueType            = T;
        using Reference            = T&;
        using ConstReference       = const T&;
        using Iterator             = ValueType*;
        using ConstIterator        = const ValueType*;
        using ReverseIterator      = ::Prism::ReverseIterator<Iterator>;
        using ConstReverseIterator = ::Prism::ReverseIterator<ConstIterator>;
        using SizeType             = usize;

        //--------------------------------------------------------------------------
        // Element Access
        //--------------------------------------------------------------------------
        constexpr Reference At(SizeType pos)
        {
            assert(pos < N);

            return Raw()[pos];
        }
        constexpr ConstReference At(SizeType pos) const
        {
            assert(pos < N);

            return Raw()[pos];
        }
        constexpr Reference operator[](SizeType pos)
        {
            assert(pos < N);

            return Raw()[pos];
        }
        constexpr ConstReference operator[](SizeType pos) const
        {
            assert(pos < N);

            return Raw()[pos];
        }
        constexpr Reference       Front() { return Raw()[0]; }
        constexpr ConstReference  Front() const { return Raw()[0]; }
        constexpr Reference       Back() { return Raw()[N - 1]; }
        constexpr ConstReference  Back() const { return Raw()[N - 1]; }
        constexpr T*              Raw() PM_NOEXCEPT { return Data; }
        constexpr const T*        Raw() const PM_NOEXCEPT { return Data; }

        //--------------------------------------------------------------------------
        // Iterators
        //--------------------------------------------------------------------------
        constexpr Iterator        begin() PM_NOEXCEPT { return Raw(); }
        constexpr ConstIterator   begin() const PM_NOEXCEPT { return Raw(); }
        constexpr ConstIterator   cbegin() const PM_NOEXCEPT { return Raw(); }

        constexpr Iterator        end() PM_NOEXCEPT { return Raw() + N; }
        constexpr ConstIterator   end() const PM_NOEXCEPT { return Raw() + N; }
        constexpr ConstIterator   cend() const PM_NOEXCEPT { return Raw() + N; }

        constexpr ReverseIterator rbegin() PM_NOEXCEPT { return Raw() + N; }
        constexpr ConstReverseIterator rbegin() const PM_NOEXCEPT
        {
            return Raw() + N;
        }
        constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT
        {
            return Raw() + N;
        }

        constexpr ReverseIterator      rend() PM_NOEXCEPT { return Raw(); }
        constexpr ConstReverseIterator rend() const PM_NOEXCEPT
        {
            return Raw();
        }
        constexpr ConstReverseIterator crend() const PM_NOEXCEPT
        {
            return Raw();
        }

        //--------------------------------------------------------------------------
        // Capacity
        //--------------------------------------------------------------------------
        constexpr bool Empty() const PM_NOEXCEPT { return begin() == end(); }
        constexpr SizeType       Size() const PM_NOEXCEPT { return N; }
        constexpr SizeType       MaxSize() const PM_NOEXCEPT { return N; }

        PM_NODISCARD constexpr T Max() const
            requires(requires(T x, T y) { x < y; })
        {
            static_assert(N > 0, "No values to max() over");

            T value = Raw()[0];
            for (usize i = 1; i < Size(); ++i)
                value = Prism::Max(Raw()[i], value);
            return value;
        }

        PM_NODISCARD constexpr T Min() const
            requires(requires(T x, T y) { x > y; })
        {
            static_assert(Size() > 0, "No values to min() over");

            T value = Raw()[0];
            for (usize i = 1; i < Size(); ++i)
                value = Prism::Min(Raw()[i], value);
            return value;
        }

        constexpr void Fill(const ValueType& value)
        {
            Memory::Fill(Raw(), value, N * sizeof(T));
        }
        constexpr void Swap(Array& other) PM_NOEXCEPT
        {
            assert(other.Size() <= N);
            for (usize i = 0; auto& v : other) Swap(v, Raw()[i++]);
        }
        constexpr auto operator<=>(const Array<T, N>& other) const = default;

        alignas(T) typename ArrayTraits<T, N>::Type Data;
    };

    template <typename T, typename... U>
    Array(T, U...)
        -> Array<EnableIfType<(IsSameV<T, U> && ...), T>, 1 + sizeof...(U)>;

    namespace Detail
    {
        template <typename T, usize N, usize... I>
        inline constexpr Array<RemoveCvType<T>, N> ToArray(T (&arr)[N],
                                                           IndexSequence<I...>)
        {
            return Array<RemoveCvType<T>, N>{{arr[I]...}};
        }
    }; // namespace Detail
    template <typename T, usize N>
    inline constexpr Array<RemoveCvType<T>, N> ToArray(T (&&arr)[N])
    {
        return Detail::ToArray(arr, MakeIndexSequence<N>{});
    }
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX != 0
using Prism::Array;
using Prism::ToArray;
#endif
