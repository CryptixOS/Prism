/*
 * Created by v1tr10l7 on 01.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

namespace Prism
{
    template <typename T = void>
    struct Less
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs < rhs;
        }
    };

    template <>
    struct Less<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) < static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct Greater
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs > rhs;
        }
    };

    template <>
    struct Greater<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) > static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct EqualTo
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs == rhs;
        }
    };

    template <>
    struct EqualTo<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) == static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct NotEqualTo
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs != rhs;
        }
    };

    template <>
    struct NotEqualTo<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) != static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct GreaterEqual
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs >= rhs;
        }
    };

    template <>
    struct GreaterEqual<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) >= static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct LessEqual
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs <= rhs;
        }
    };

    template <>
    struct LessEqual<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) <= static_cast<U&&>(rhs);
        }
    };
} // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::EqualTo;
using Prism::Greater;
using Prism::GreaterEqual;
using Prism::Less;
using Prism::LessEqual;
using Prism::NotEqualTo;
#endif
