/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Deque.hpp>

namespace Prism
{
    template <typename T, typename Container = Deque<T>>
    class Queue
    {
      public:
        using ContainerType  = Container;
        using ValueType      = Container::ValueType;
        using Reference      = Container::Reference;
        using ConstReference = Container::ConstReference;

        inline constexpr Queue()
            : Queue(Container())
        {
        }
        explicit inline constexpr Queue(const Container& container);
        explicit inline constexpr Queue(Container&& container);
        template <typename InputIt>
        inline constexpr Queue(InputIt first, InputIt last);

        inline constexpr ~Queue();

        inline constexpr Queue&         operator=(const Queue& other);
        inline constexpr Queue&         operator=(Queue&& other);

        inline constexpr Reference      Front();
        inline constexpr ConstReference Front() const;

        inline constexpr Reference      Back();
        inline constexpr ConstReference Back() const;

        inline constexpr bool           Empty() const;
        inline constexpr usize          Size() const;

        inline constexpr void           Push(const ValueType& value);
        inline constexpr void           Push(ValueType&& value);

        template <typename... Args>
        inline constexpr decltype(auto) Emplace(Args&&... args);

        inline constexpr void           Pop();
        inline constexpr ValueType      PopElement();
        inline constexpr void           Swap(Queue& other) PM_NOEXCEPT;

      private:
        Container m_Container;
    };

    template <typename T, typename Container>
    inline constexpr void Swap(Queue<T, Container>& lhs,
                               Queue<T, Container>& rhs) PM_NOEXCEPT
        requires(IsSwappableV<Container>);
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Queue;
#endif

#include <Prism/Containers/Queue.inl>
