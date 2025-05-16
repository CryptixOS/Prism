/*
 * Created by v1tr10l7 on 12.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Debug/Assertions.hpp>

#include <Prism/Memory/Ref.hpp>

namespace Prism
{
    template <typename T>
    class SplitBuffer
    {
      public:
        SplitBuffer(usize capacity = 16)
            : m_Capacity(capacity)
            , m_Size(0)
            , m_GapStart(0)
            , m_GapEnd(capacity)
            , m_Data(CreateRef<T[]>(capacity))
        {
        }

        usize    Size() const { return m_Size; }

        const T& operator[](usize index) const
        {
            assert(index < m_Size);
            return index < m_GapStart ? m_Data[index]
                                      : m_Data[index + (m_GapEnd - m_GapStart)];
        }

        T& operator[](usize index)
        {
            assert(index < m_Size);
            return index < m_GapStart ? m_Data[index]
                                      : m_Data[index + (m_GapEnd - m_GapStart)];
        }

        void Insert(usize index, const T& value)
        {
            assert(index <= m_Size);
            MoveGapTo(index);
            if (m_GapStart == m_GapEnd) Grow();
            m_Data[m_GapStart++] = value;
            ++m_Size;
        }

        void Erase(usize index)
        {
            assert(index < m_Size);
            MoveGapTo(index);
            --m_GapEnd;
            --m_Size;
        }

        class Iterator
        {
          public:
            using iterator_category = RandomAccessIteratorTag;
            using value_type        = T;
            using difference_type   = ptrdiff;
            using pointer           = T*;
            using reference         = T&;

            Iterator(SplitBuffer* buffer, usize index)
                : m_Buffer(buffer)
                , m_Index(index)
            {
            }

            reference operator*() const { return (*m_Buffer)[m_Index]; }
            pointer   operator->() const { return &(*m_Buffer)[m_Index]; }

            Iterator& operator++()
            {
                ++m_Index;
                return *this;
            }
            Iterator operator++(int)
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }
            Iterator& operator--()
            {
                --m_Index;
                return *this;
            }
            Iterator operator--(int)
            {
                Iterator tmp = *this;
                --(*this);
                return tmp;
            }

            Iterator& operator+=(difference_type n)
            {
                m_Index += n;
                return *this;
            }
            Iterator operator+(difference_type n) const
            {
                return Iterator(m_Buffer, m_Index + n);
            }
            Iterator& operator-=(difference_type n)
            {
                m_Index -= n;
                return *this;
            }
            Iterator operator-(difference_type n) const
            {
                return Iterator(m_Buffer, m_Index - n);
            }
            difference_type operator-(const Iterator& other) const
            {
                return m_Index - other.m_Index;
            }

            bool operator==(const Iterator& other) const
            {
                return m_Index == other.m_Index;
            }
            bool operator!=(const Iterator& other) const
            {
                return m_Index != other.m_Index;
            }
            bool operator<(const Iterator& other) const
            {
                return m_Index < other.m_Index;
            }
            bool operator>(const Iterator& other) const
            {
                return m_Index > other.m_Index;
            }
            bool operator<=(const Iterator& other) const
            {
                return m_Index <= other.m_Index;
            }
            bool operator>=(const Iterator& other) const
            {
                return m_Index >= other.m_Index;
            }

          private:
            SplitBuffer* m_Buffer;
            usize        m_Index;
        };

        Iterator Begin() { return Iterator(this, 0); }
        Iterator End() { return Iterator(this, m_Size); }

      private:
        usize    m_Capacity;
        usize    m_Size;
        usize    m_GapStart;
        usize    m_GapEnd;
        Ref<T[]> m_Data;

        void     MoveGapTo(usize index)
        {
            if (index < m_GapStart)
            {
                usize moveCount = m_GapStart - index;
                for (usize i = 0; i < moveCount; ++i)
                    m_Data[m_GapEnd - 1 - i] = Move(m_Data[m_GapStart - 1 - i]);
            }
            else if (index > m_GapStart)
            {
                usize moveCount = index - m_GapStart;
                for (usize i = 0; i < moveCount; ++i)
                    m_Data[m_GapStart + i] = Move(m_Data[m_GapEnd + i]);
            }
            m_GapEnd += m_GapStart - index;
            m_GapStart = index;
        }

        void Grow()
        {
            usize    newCapacity = m_Capacity * 2;
            Ref<T[]> newData     = CreateRef<T[]>(newCapacity);

            for (usize i = 0; i < m_GapStart; ++i) newData[i] = Move(m_Data[i]);

            usize backCount = m_Capacity - m_GapEnd;
            for (usize i = 0; i < backCount; ++i)
                newData[newCapacity - backCount + i]
                    = Move(m_Data[m_GapEnd + i]);

            m_GapEnd   = newCapacity - backCount;
            m_Data     = Move(newData);
            m_Capacity = newCapacity;
        }
    };
}; // namespace Prism
