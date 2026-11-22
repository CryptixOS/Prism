/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Containers.CircularQueue;

export import Prism.Core.Iterator;
export import Prism.Core.TypeTraits;

export extern "C++"
{
    namespace Prism
    {
        /**
         * @brief A fixed-capacity FIFO queue in inline storage. Pushing onto a
         * full queue drops the oldest element, so it always holds the last
         * `Capacity` elements pushed (a ring buffer of recent events).
         *
         * Index 0 / Front() is the oldest element, Back() the newest.
         */
        template <typename T, usize Capacity>
        class CircularQueue
        {
            static_assert(Capacity > 0, "CircularQueue needs room for an element");

            template <bool Const>
            class BasicIterator
            {
                using Queue = ConditionalType<Const, const CircularQueue,
                                              CircularQueue>;

              public:
                using ValueType = T;
                using Reference = ConditionalType<Const, const T&, T&>;

                constexpr Reference operator*() const { return (*m_Queue)[m_Index]; }
                constexpr auto*     operator->() const { return &**this; }
                constexpr BasicIterator& operator++()
                {
                    ++m_Index;
                    return *this;
                }
                constexpr BasicIterator operator++(int)
                {
                    BasicIterator copy = *this;
                    ++m_Index;
                    return copy;
                }
                constexpr bool operator==(const BasicIterator& other) const
                {
                    return m_Queue == other.m_Queue && m_Index == other.m_Index;
                }

              private:
                friend class CircularQueue;
                constexpr BasicIterator(Queue* queue, usize index)
                    : m_Queue(queue)
                    , m_Index(index)
                {
                }

                Queue* m_Queue = nullptr;
                usize  m_Index = 0;
            };

          public:
            using ValueType     = T;
            using Iterator      = BasicIterator<false>;
            using ConstIterator = BasicIterator<true>;

            constexpr CircularQueue() = default;
            CircularQueue(const CircularQueue& other)
            {
                for (const T& value : other) Push(value);
            }
            CircularQueue(CircularQueue&& other)
            {
                for (T& value : other) Push(Move(value));
                other.Clear();
            }
            CircularQueue& operator=(const CircularQueue& other)
            {
                if (this == &other) return *this;
                Clear();
                for (const T& value : other) Push(value);
                return *this;
            }
            CircularQueue& operator=(CircularQueue&& other)
            {
                if (this == &other) return *this;
                Clear();
                for (T& value : other) Push(Move(value));
                other.Clear();
                return *this;
            }
            ~CircularQueue() { Clear(); }

            static constexpr usize MaxSize() { return Capacity; }
            constexpr usize        Size() const { return m_Size; }
            constexpr bool         Empty() const { return m_Size == 0; }
            constexpr bool         Full() const { return m_Size == Capacity; }

            /// The pos-th oldest element.
            T&                     operator[](usize pos)
            {
                PrismCheck(pos < m_Size);
                return *Slot((m_Head + pos) % Capacity);
            }
            const T& operator[](usize pos) const
            {
                PrismCheck(pos < m_Size);
                return *Slot((m_Head + pos) % Capacity);
            }
            T&       At(usize pos) { return (*this)[pos]; }
            const T& At(usize pos) const { return (*this)[pos]; }

            T&       Front() { return (*this)[0]; }
            const T& Front() const { return (*this)[0]; }
            T&       Back() { return (*this)[m_Size - 1]; }
            const T& Back() const { return (*this)[m_Size - 1]; }

            Iterator begin() { return Iterator(this, 0); }
            Iterator end() { return Iterator(this, m_Size); }
            ConstIterator begin() const { return ConstIterator(this, 0); }
            ConstIterator end() const { return ConstIterator(this, m_Size); }
            ConstIterator cbegin() const { return begin(); }
            ConstIterator cend() const { return end(); }

            void          Clear()
            {
                while (!Empty()) (void)Pop();
                m_Head = 0;
            }

            /// Appends a value, dropping the oldest one if the queue is full.
            template <typename... Args>
            T& Emplace(Args&&... args)
            {
                if (Full()) (void)Pop();
                T* element = new (Slot((m_Head + m_Size) % Capacity))
                    T(Forward<Args>(args)...);
                ++m_Size;
                return *element;
            }
            void Push(const T& value) { Emplace(value); }
            void Push(T&& value) { Emplace(Move(value)); }
            template <typename... Args>
            void PushInPlace(Args&&... args)
            {
                Emplace(Forward<Args>(args)...);
            }

            /// Removes and returns the oldest element.
            T Pop()
            {
                PrismCheck(!Empty());
                T* element = Slot(m_Head);
                T  value   = Move(*element);
                element->~T();

                m_Head = (m_Head + 1) % Capacity;
                --m_Size;
                return value;
            }
            T PopFront() { return Pop(); }
            /// Removes and returns the newest element.
            T PopBack()
            {
                PrismCheck(!Empty());
                T* element = Slot((m_Head + m_Size - 1) % Capacity);
                T  value   = Move(*element);
                element->~T();

                --m_Size;
                return value;
            }

          private:
            alignas(T) u8 m_Storage[sizeof(T) * Capacity];
            usize m_Head = 0;
            usize m_Size = 0;

            T*    Slot(usize index)
            {
                return __builtin_launder(reinterpret_cast<T*>(m_Storage)
                                         + index);
            }
            const T* Slot(usize index) const
            {
                return __builtin_launder(reinterpret_cast<const T*>(m_Storage)
                                         + index);
            }
        };
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::CircularQueue;
#endif
}
