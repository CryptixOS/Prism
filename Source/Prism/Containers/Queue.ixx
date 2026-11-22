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

export module Prism.Containers.Queue;

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

#if PRISM_USE_NAMESPACE != 0
    using Prism::Queue;
#endif


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
}
