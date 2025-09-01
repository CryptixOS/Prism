/*
 * Created by v1tr10l7 on 01.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    /**
     *  @brief  Return the Size of a container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    Size(const Container& container) PM_NOEXCEPT(PM_NOEXCEPT(container.Size()))
        -> decltype(container.Size())
    {
        return container.Size();
    }

    /**
     *  @brief  Return the Size of an array.
     */
    template <typename T, usize Count>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr usize
    Size(const T (&)[Count]) PM_NOEXCEPT
    {
        return Count;
    }

    /**
     *  @brief  Return whether a container is Empty.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    Empty(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.Empty()))
            -> decltype(container.Empty())
    {
        return container.Empty();
    }

    /**
     *  @brief  Return whether an array is Empty (always false).
     */
    template <typename T, usize Count>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr bool
    Empty(const T (&)[Count]) PM_NOEXCEPT
    {
        return false;
    }

    /**
     *  @brief  Return whether an InitializerList is Empty.
     *  @param  ilist  Initializer list.
     */
    template <typename T>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr bool
    Empty(InitializerList<T> ilist) PM_NOEXCEPT
    {
        return ilist.size() == 0;
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Empty;
using Prism::Size;
#endif
