/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T, usize Capacity>
    class CircularQueue
    {
        class Iterator
        {
          public:
            bool operator!=(Iterator const& other)
            {
                return m_Index != other.m_Index;
            }
            Iterator& operator++()
            {
                ++m_Index;
                return *this;
            }

            T& operator*() const { return m_Queue.At(m_Index); }

          private:
            friend class CircularQueue;
            Iterator(CircularQueue& queue, size_t const index)
                : m_Queue(queue)
                , m_Index(index)
            {
            }
            CircularQueue& m_Queue;
            size_t         m_Index = 0;
        };

      public:
        CircularQueue() = default;
        ~CircularQueue() { Clear(); }

        T&       At(usize pos) { return m_Storage[(m_Head + pos) % Capacity]; }
        const T& At(usize pos) const
        {
            return m_Storage[(m_Head + pos) % Capacity];
        }

        T& operator[](usize pos)
        {
            return m_Storage[(m_Head + pos) % Capacity];
        }
        const T& operator[](usize pos) const
        {
            return m_Storage[(m_Head + pos) % Capacity];
        }

        T&             Front() { return m_Storage[0]; }
        const T&       Front() const { return m_Storage[0]; }

        T&             Back() { return m_Storage[Capacity - 1]; }
        const T&       Back() const { return m_Storage[Capacity - 1]; }

        Iterator       begin() { return Iterator(*this, 0); }
        const Iterator begin() const { return Iterator(*this, 0); }
        const Iterator cbegin() const PM_NOEXCEPT { return Iterator(*this, 0); }

        Iterator       end() { return Iterator(*this, m_Size); }
        const Iterator end() const { return Iterator(*this, m_Size); }
        const Iterator cend() const PM_NOEXCEPT
        {
            return Iterator(*this, m_Size);
        }

        Prism::ReverseIterator<Iterator> rbegin()
        {
            return Iterator(*this, m_Size);
        }
        Prism::ReverseIterator<const Iterator> rbegin() const
        {
            return Iterator(*this, m_Size);
        }
        Prism::ReverseIterator<const Iterator> rcbegin() const PM_NOEXCEPT
        {
            return Iterator(*this, m_Size);
        }

        Prism::ReverseIterator<Iterator> rend() { return Iterator(*this, 0); }
        Prism::ReverseIterator<const Iterator> rend() const
        {
            return Iterator(*this, 0);
        }
        Prism::ReverseIterator<const Iterator> rcend() const PM_NOEXCEPT
        {
            return Iterator(*this, 0);
        }

        bool  Empty() const { return m_Size == 0; }
        usize Size() const { return m_Size; }

        void  Clear()
        {
            for (usize i = 0; i < m_Size; ++i)
                m_Storage[(m_Head + i) % Capacity].~T();

            m_Head = 0;
            m_Size = 0;
        }

        void Push(T&& value)
        {
            auto& element = m_Storage[(m_Head + m_Size) % Capacity];
            if (m_Size == Capacity) element.~T();

            new (&element) T(Forward<T>(value));
            if (m_Size == Capacity) m_Head = (m_Head + 1) % Capacity;
            else ++m_Size;
        }
        template <typename... Args>
        void PushInPlace(Args&&... args)
        {
            auto& element = m_Storage[(m_Head + m_Size) % Capacity];
            if (m_Size == Capacity) element.~T();

            new (&element) T(Forward<Args...>((args)...));
            if (m_Size == Capacity) m_Head = (m_Head + 1) % Capacity;
            else ++m_Size;
        }

        T Pop()
        {
            assert(!Empty());
            auto& element = m_Storage[m_Head];
            T     value   = Move(element);
            element.~T();

            m_Head = (m_Head + 1) % Capacity;
            --m_Size;
            return value;
        }
        inline T PopFront()
        {
            assert(!Empty());
            m_Head = m_Head == 0 ? Capacity - 1 : (m_Head - 1) % Capacity;
            auto& element = m_Storage[(m_Head + m_Size - 1) % Capacity];
            T     value   = Move(element);
            element.~T();

            --m_Size;
            return value;
        }

      private:
        u8    m_Storage[sizeof(T) * Capacity];
        usize m_Head = 0;
        usize m_Size = 0;
    };
}; // namespace Prism

#ifdef PRISM_TARGET_CRYPTIX
using Prism::CircularQueue;
#endif
