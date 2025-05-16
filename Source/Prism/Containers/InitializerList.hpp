/*
 * Created by v1tr10l7 on 09.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Concepts.hpp>
#include <Prism/Core/Compiler.hpp>

namespace Prism
{
    template <typename T>
    class InitializerList
    {
      public:
        using ValueType                         = T;
        using Reference                         = const T&;
        using ConstReference                    = const T&;
        using SizeType                          = usize;
        using Iterator                          = const T*;
        using ConstIterator                     = const T*;

        constexpr InitializerList() PM_NOEXCEPT = default;

        constexpr SizeType Size() const PM_NOEXCEPT { return m_Size; }

        constexpr Iterator begin() const PM_NOEXCEPT { return m_Data; }
        constexpr Iterator end() const PM_NOEXCEPT { return m_Data + m_Size; }

      private:
        Iterator m_Data = nullptr;
        SizeType m_Size = 0;

        constexpr InitializerList(ConstIterator list, SizeType size)
            : m_Data(list)
            , m_Size(size)
        {
        }
    };

    /**
     *  @brief  Return an iterator pointing to the first element of
     *          the initializer_list.
     *  @param  il  Initializer list.
     *  @relates initializer_list
     */
    template <typename T>
    constexpr const T* begin(InitializerList<T> il) PM_NOEXCEPT
    {
        return il.begin();
    }

    /**
     *  @brief  Return an iterator pointing to one past the last element
     *          of the initializer_list.
     *  @param  il  Initializer list.
     *  @relates initializer_list
     */
    template <typename T>
    constexpr const T* end(InitializerList<T> il) PM_NOEXCEPT
    {
        return il.end();
    }

}; // namespace Prism
