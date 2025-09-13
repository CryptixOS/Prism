/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Stack.hpp>

namespace Prism
{
    template <typename T, typename Container>
    inline constexpr Stack<T, Container>::Stack(const Container& container)
        : m_Container(container)
    {
    }
    template <typename T, typename Container>
    inline constexpr Stack<T, Container>::Stack(Container&& container)
        : m_Container(Move(container))
    {
    }
    template <typename T, typename Container>
    template <typename InputIt>
    inline constexpr Stack<T, Container>::Stack(InputIt first, InputIt last)
    {
        InputIt current = first;
        while (current != last) m_Container.PushBack(*current++);
    }

    template <typename T, typename Container>
    inline constexpr Stack<T, Container>::~Stack()
    {
    }

    template <typename T, typename Container>
    inline constexpr Stack<T, Container>&
    Stack<T, Container>::operator=(const Stack& other)
    {
        m_Container = other.m_Container;

        return *this;
    }
    template <typename T, typename Container>
    inline constexpr Stack<T, Container>&
    Stack<T, Container>::operator=(Stack&& other)
    {
        m_Container = Move(other.m_Container);

        return *this;
    }

    template <typename T, typename Container>
    inline constexpr Container::Reference Stack<T, Container>::Front()
    {
        return m_Container.Front();
    }
    template <typename T, typename Container>
    inline constexpr Container::ConstReference
    Stack<T, Container>::Front() const
    {
        return m_Container.Front();
    }

    template <typename T, typename Container>
    inline constexpr Container::Reference Stack<T, Container>::Back()
    {
        return m_Container.Back();
    }
    template <typename T, typename Container>
    inline constexpr Container::ConstReference Stack<T, Container>::Back() const
    {
        return m_Container.Back();
    }

    template <typename T, typename Container>
    inline constexpr bool Stack<T, Container>::Empty() const
    {
        return m_Container.Empty();
    }
    template <typename T, typename Container>
    inline constexpr usize Stack<T, Container>::Size() const
    {
        return m_Container.Size();
    }
    template <typename T, typename Container>
    inline constexpr void Stack<T, Container>::Push(const ValueType& value)
    {
        m_Container.PushBack(value);
    }
    template <typename T, typename Container>
    inline constexpr void Stack<T, Container>::Push(ValueType&& value)
    {
        m_Container.PushBack(Move(value));
    }

    template <typename T, typename Container>
    template <typename... Args>
    inline constexpr decltype(auto) Stack<T, Container>::Emplace(Args&&... args)
    {
        return m_Container.EmplaceBack(Forward<Args>(args)...);
    }

    template <typename T, typename Container>
    inline constexpr void Stack<T, Container>::Pop()
    {
        m_Container.PopBack();
    }
    template <typename T, typename Container>
    inline constexpr Stack<T, Container>::ValueType
    Stack<T, Container>::PopElement()
    {
        return m_Container.PopBackElement();
    }
    template <typename T, typename Container>
    inline constexpr void Stack<T, Container>::Swap(Stack& other) PM_NOEXCEPT
    {
        return m_Container.Swap(other.m_Container);
    }

    template <typename T, typename Container>
    inline constexpr void Swap(Stack<T, Container>& lhs,
                               Stack<T, Container>& rhs) PM_NOEXCEPT
        requires(IsSwappableV<Container>)
    {
        lhs.Swap(rhs);
    }
}; // namespace Prism
