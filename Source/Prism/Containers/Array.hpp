/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <utility>

namespace Prism
{
    template <typename T, usize N>
    struct Array
    {
        using ValueType            = T;
        using Reference            = T&;
        using ConstReference       = const T&;
        using Iterator             = ValueType*;
        using ConstIterator        = const ValueType*;
        using ReverseIterator      = std::reverse_iterator<Iterator>;
        using ConstReverseIterator = std::reverse_iterator<ConstIterator>;
        using SizeType             = usize;

        //--------------------------------------------------------------------------
        // Element Access
        //--------------------------------------------------------------------------
        constexpr Reference At(SizeType pos)
        {
            assert(pos < N);

            return Data[pos];
        }
        constexpr ConstReference At(SizeType pos) const
        {
            assert(pos < N);

            return Data[pos];
        }
        constexpr Reference operator[](SizeType pos)
        {
            assert(pos < N);

            return Data[pos];
        }
        constexpr ConstReference operator[](SizeType pos) const
        {
            assert(pos < N);

            return Data[pos];
        }
        constexpr Reference       Front() { return Data[0]; }
        constexpr ConstReference  Front() const { return Data[0]; }
        constexpr Reference       Back() { return Data[N - 1]; }
        constexpr ConstReference  Back() const { return Data[N - 1]; }
        constexpr T*              Raw() PM_NOEXCEPT { return Data; }
        constexpr const T*        Raw() const PM_NOEXCEPT { return Data; }

        //--------------------------------------------------------------------------
        // Iterators
        //--------------------------------------------------------------------------
        constexpr Iterator        begin() PM_NOEXCEPT { return Data; }
        constexpr ConstIterator   begin() const PM_NOEXCEPT { return Data; }
        constexpr ConstIterator   cbegin() const PM_NOEXCEPT { return Data; }

        constexpr Iterator        end() PM_NOEXCEPT { return Data + N; }
        constexpr ConstIterator   end() const PM_NOEXCEPT { return Data + N; }
        constexpr ConstIterator   cend() const PM_NOEXCEPT { return Data + N; }

        constexpr ReverseIterator rbegin() PM_NOEXCEPT { return Data + N; }
        constexpr ConstReverseIterator rbegin() const PM_NOEXCEPT
        {
            return Data + N;
        }
        constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT
        {
            return Data + N;
        }

        constexpr ReverseIterator      rend() PM_NOEXCEPT { return Data; }
        constexpr ConstReverseIterator rend() const PM_NOEXCEPT { return Data; }
        constexpr ConstReverseIterator crend() const PM_NOEXCEPT
        {
            return Data;
        }

        //--------------------------------------------------------------------------
        // Capacity
        //--------------------------------------------------------------------------
        constexpr bool Empty() const PM_NOEXCEPT { return begin() == end(); }
        constexpr SizeType Size() const PM_NOEXCEPT { return N; }
        constexpr SizeType MaxSize() const PM_NOEXCEPT { return N; }

        constexpr void     Fill(const ValueType& value)
        {
            std::memset(Data, value, N * sizeof(T));
        }
        constexpr void Swap(Array& other) PM_NOEXCEPT
        {
            assert(other.Size() <= N);
            for (usize i = 0; auto& v : other) std::swap(v, Data[i++]);
        }
        constexpr auto operator<=>(const Array<T, N>& other) const = default;

        T              Data[N];
    };

    namespace Detail
    {
        template <typename T, usize N, usize... I>
        inline constexpr Array<std::remove_cv_t<T>, N>
        ToArray(T (&arr)[N], std::index_sequence<I...>)
        {
            return Array<std::remove_cv_t<T>, N>{{arr[I]...}};
        }
    }; // namespace Detail
    template <typename T, usize N>
    inline constexpr Array<std::remove_cv_t<T>, N> ToArray(T (&&arr)[N])
    {
        return Detail::ToArray(arr, std::make_index_sequence<N>{});
    }
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::Array;
using Prism::ToArray;
#endif
