/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Assertions.hpp>

export module Prism.Containers.Stack;

export import Prism.Containers.Deque;
export import Prism.Core.Iterator;
export import Prism.Core.Limits;
export import Prism.Memory.Memory;
export import Prism.Core.Core;
export import Prism.Debug.Assertions;
export import Prism.Memory.Ref;

export extern "C++"
{
    namespace Prism
    {
        template <typename T, typename Container = Deque<T>>
        class Stack
        {
          public:
            using ContainerType  = Container;
            using ValueType      = Container::ValueType;
            using Reference      = Container::Reference;
            using ConstReference = Container::ConstReference;

            inline constexpr Stack()
                : Stack(Container())
            {
            }
            explicit inline constexpr Stack(const Container& container);
            explicit inline constexpr Stack(Container&& container);
            template <typename InputIt>
            inline constexpr Stack(InputIt first, InputIt last);

            inline constexpr ~Stack();

            inline constexpr Stack&         operator=(const Stack& other);
            inline constexpr Stack&         operator=(Stack&& other);

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
            inline constexpr void           Swap(Stack& other) PM_NOEXCEPT;

          private:
            Container m_Container;
        };

        template <typename T, typename Container>
        inline constexpr void Swap(Stack<T, Container>& lhs,
                                   Stack<T, Container>& rhs) PM_NOEXCEPT
            requires(IsSwappableV<Container>);
    }; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
    using Prism::Stack;
#endif


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
}
