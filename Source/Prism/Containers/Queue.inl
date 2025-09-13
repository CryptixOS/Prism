/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Queue.hpp>

namespace Prism
{
    template <typename T, typename Container>
    inline constexpr Queue<T, Container>::Queue(const Container& container)
        : m_Container(container)
    {
    }
    template <typename T, typename Container>
    inline constexpr Queue<T, Container>::Queue(Container&& container)
        : m_Container(Move(container))
    {
    }
    template <typename T, typename Container>
    template <typename InputIt>
    inline constexpr Queue<T, Container>::Queue(InputIt first, InputIt last)
    {
        InputIt current = first;
        while (current != last) m_Container.PushBack(*current++);
    }

    template <typename T, typename Container>
    inline constexpr Queue<T, Container>::~Queue()
    {
    }

    template <typename T, typename Container>
    inline constexpr Queue<T, Container>&
    Queue<T, Container>::operator=(const Queue& other)
    {
        m_Container = other.m_Container;

        return *this;
    }
    template <typename T, typename Container>
    inline constexpr Queue<T, Container>&
    Queue<T, Container>::operator=(Queue&& other)
    {
        m_Container = Move(other.m_Container);

        return *this;
    }

    template <typename T, typename Container>
    inline constexpr Container::Reference Queue<T, Container>::Front()
    {
        return m_Container.Front();
    }
    template <typename T, typename Container>
    inline constexpr Container::ConstReference
    Queue<T, Container>::Front() const
    {
        return m_Container.Front();
    }

    template <typename T, typename Container>
    inline constexpr Container::Reference Queue<T, Container>::Back()
    {
        return m_Container.Back();
    }
    template <typename T, typename Container>
    inline constexpr Container::ConstReference Queue<T, Container>::Back() const
    {
        return m_Container.Back();
    }

    template <typename T, typename Container>
    inline constexpr bool Queue<T, Container>::Empty() const
    {
        return m_Container.Empty();
    }
    template <typename T, typename Container>
    inline constexpr usize Queue<T, Container>::Size() const
    {
        return m_Container.Size();
    }
    template <typename T, typename Container>
    inline constexpr void Queue<T, Container>::Push(const ValueType& value)
    {
        m_Container.PushBack(value);
    }
    template <typename T, typename Container>
    inline constexpr void Queue<T, Container>::Push(ValueType&& value)
    {
        m_Container.PushBack(Move(value));
    }

    template <typename T, typename Container>
    template <typename... Args>
    inline constexpr decltype(auto) Queue<T, Container>::Emplace(Args&&... args)
    {
        return m_Container.EmplaceBack(Forward<Args>(args)...);
    }

    template <typename T, typename Container>
    inline constexpr void Queue<T, Container>::Pop()
    {
        m_Container.PopFront();
    }
    template <typename T, typename Container>
    inline constexpr Queue<T, Container>::ValueType
    Queue<T, Container>::PopElement()
    {
        return m_Container.PopFrontElement();
    }
    template <typename T, typename Container>
    inline constexpr void Queue<T, Container>::Swap(Queue& other) PM_NOEXCEPT
    {
        return m_Container.Swap(other.m_Container);
    }

    template <typename T, typename Container>
    inline constexpr void Swap(Queue<T, Container>& lhs,
                               Queue<T, Container>& rhs) PM_NOEXCEPT
        requires(IsSwappableV<Container>)
    {
        lhs.Swap(rhs);
    }
}; // namespace Prism
