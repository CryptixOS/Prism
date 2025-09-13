/*
 * Created by v1tr10l7 on 30.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Core.hpp>
#include <Prism/Core/Iterator.hpp>

#include <Prism/Debug/Assertions.hpp>
#include <Prism/Memory/Ref.hpp>

namespace Prism
{
    template <typename T>
    class Block : public RefCounted
    {
      public:
        static constexpr usize BlockSize = 64;

        Block()                          = default;

        T&       operator[](usize i) { return m_Data[i]; }
        const T& operator[](usize i) const { return m_Data[i]; }

      private:
        T m_Data[BlockSize];
    };

    template <typename T>
    class Deque
    {
      public:
        constexpr Deque()
        {
            m_Blocks.PushBack(BlockRef::Create());
            m_FrontIndex = 0;
            m_BackIndex  = 0;
        }

        template <bool IsConst = false>
        class Iterator
        {
          public:
            using BlockType    = Block<T>;
            using BlockRefType = ConditionalType<IsConst, const Ref<BlockType>,
                                                 Ref<BlockType>>;
            using ValueType    = ConditionalType<IsConst, const T, T>;
            using Pointer      = ValueType*;
            using Reference    = ValueType&;
            using IteratorCategory = RandomAccessIteratorTag;
            using DifferenceType   = ptrdiff;

            constexpr Iterator(Vector<BlockRefType>* blocks, usize block,
                               usize offset)
                : m_Blocks(blocks)
                , m_BlockIndex(block)
                , m_Offset(offset)
            {
            }

            constexpr Reference operator*() const
            {
                return (*(*m_Blocks)[m_BlockIndex])[m_Offset];
            }

            constexpr Pointer operator->() const
            {
                return &(*(*m_Blocks)[m_BlockIndex])[m_Offset];
            }

            constexpr Reference operator[](DifferenceType n) const
            {
                return *(*this + n);
            }

            constexpr Iterator& operator++()
            {
                if (++m_Offset == Block<T>::BlockSize)
                {
                    m_Offset = 0;
                    ++m_BlockIndex;
                }
                return *this;
            }
            constexpr Iterator operator++(int)
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr Iterator& operator--()
            {
                if (m_Offset == 0)
                {
                    --m_BlockIndex;
                    m_Offset = Block<T>::BlockSize - 1;
                }
                else { --m_Offset; }
                return *this;
            }
            constexpr Iterator operator--(int)
            {
                Iterator tmp = *this;
                --(*this);
                return tmp;
            }

            constexpr Iterator operator+(DifferenceType n) const
            {
                Iterator result = *this;
                result += n;
                return result;
            }

            constexpr Iterator& operator+=(DifferenceType n)
            {
                DifferenceType flatIndex
                    = static_cast<DifferenceType>(m_BlockIndex)
                        * Block<T>::BlockSize
                    + static_cast<DifferenceType>(m_Offset) + n;
                m_BlockIndex = flatIndex / Block<T>::BlockSize;
                m_Offset     = flatIndex % Block<T>::BlockSize;
                return *this;
            }

            constexpr Iterator operator-(DifferenceType n) const
            {
                return *this + (-n);
            }

            constexpr Iterator& operator-=(DifferenceType n)
            {
                return *this += -n;
            }

            constexpr DifferenceType operator-(const Iterator& other) const
            {
                return static_cast<DifferenceType>(
                    (m_BlockIndex - other.m_BlockIndex) * Block<T>::BlockSize
                    + (m_Offset - other.m_Offset));
            }

            constexpr bool operator==(const Iterator& other) const = default;

            constexpr auto operator<=>(const Iterator& other) const
            {
                if (m_BlockIndex != other.m_BlockIndex)
                    return m_BlockIndex <=> other.m_BlockIndex;
                return m_Offset <=> other.m_Offset;
            }

          private:
            Vector<BlockRefType>* m_Blocks;
            usize                 m_BlockIndex;
            usize                 m_Offset;

            template <typename, bool>
            friend class DequeIterator;
            template <typename>
            friend struct ReverseIterator;
            friend class Deque<T>;
        };

        using ValueType            = T;
        using Reference            = T&;
        using ConstReference       = const T&;
        using ConstIterator        = Iterator<true>;
        using ReverseIterator      = ::Prism::ReverseIterator<Iterator<false>>;
        using ConstReverseIterator = ::Prism::ReverseIterator<Iterator<true>>;

        constexpr T& operator[](usize index)
        {
            usize flatIndex  = m_FrontOffset + index;
            usize blockIndex = flatIndex / BlockSize;
            usize offset     = flatIndex % BlockSize;
            return (*m_Blocks[m_FrontIndex + blockIndex])[offset];
        }

        constexpr const T& operator[](usize index) const
        {
            usize flatIndex  = m_FrontOffset + index;
            usize blockIndex = flatIndex / BlockSize;
            usize offset     = flatIndex % BlockSize;
            return (*m_Blocks[m_FrontIndex + blockIndex])[offset];
        }

        constexpr void PushBack(const T& value)
        {
            if (m_BackOffset == BlockSize)
            {
                if (m_BackIndex + 1 >= m_Blocks.Size())
                    m_Blocks.PushBack(BlockRef::Create());
                ++m_BackIndex;
                m_BackOffset = 0;
            }

            (*m_Blocks[m_BackIndex])[m_BackOffset++] = value;
        }
        template <typename... Args>
        constexpr void EmplaceBack(Args&&... args)
        {
            T value(Forward<Args>(args)...);
            if (m_BackOffset == BlockSize)
            {
                if (m_BackIndex + 1 >= m_Blocks.Size())
                    m_Blocks.PushBack(BlockRef::Create());
                ++m_BackIndex;
                m_BackOffset = 0;
            }

            (*m_Blocks[m_BackIndex])[m_BackOffset++] = value;
        }

        constexpr void PushFront(const T& value)
        {
            if (m_FrontOffset == 0)
            {
                if (m_FrontIndex == 0)
                {
                    m_Blocks.Insert(m_Blocks.begin(), BlockRef::Create());
                    ++m_BackIndex;
                }
                else { --m_FrontIndex; }
                m_FrontOffset = BlockSize;
            }

            (*m_Blocks[m_FrontIndex])[--m_FrontOffset] = value;
        }
        template <typename... Args>
        constexpr void EmplaceFront(Args&&... args)
        {
            T value(Forward<Args>(args)...);
            if (m_FrontOffset == 0)
            {
                if (m_FrontIndex == 0)
                {
                    m_Blocks.Insert(m_Blocks.begin(), BlockRef::Create());
                    ++m_BackIndex;
                }
                else { --m_FrontIndex; }
                m_FrontOffset = BlockSize;
            }

            (*m_Blocks[m_FrontIndex])[--m_FrontOffset] = value;
        }

        constexpr T PopBackElement()
        {
            if (m_BackOffset == 0)
            {
                --m_BackIndex;
                m_BackOffset = BlockSize;
            }
            return (*m_Blocks[m_BackIndex])[--m_BackOffset];
        }
        constexpr void PopBack()
        {
            if (m_BackOffset == 0)
            {
                --m_BackIndex;
                m_BackOffset = BlockSize;
            }
            --m_BackOffset;
        }

        constexpr T PopFrontElement()
        {
            T value = (*m_Blocks[m_FrontIndex])[m_FrontOffset++];
            if (m_FrontOffset == BlockSize)
            {
                ++m_FrontIndex;
                m_FrontOffset = 0;
            }
            return value;
        }
        constexpr void PopFront()
        {
            ++m_FrontOffset;
            if (m_FrontOffset == BlockSize)
            {
                ++m_FrontIndex;
                m_FrontOffset = 0;
            }
        }

        constexpr T& Front()
        {
            return (*m_Blocks[m_FrontIndex])[m_FrontOffset];
        }
        constexpr const T& Front() const
        {
            return (*m_Blocks[m_FrontIndex])[m_FrontOffset];
        }

        constexpr T& Back()
        {
            return (*m_Blocks[m_BackIndex])[m_BackOffset - 1];
        }
        constexpr const T& Back() const
        {
            return (*m_Blocks[m_BackIndex])[m_BackOffset - 1];
        }

        constexpr Iterator<> begin()
        {
            return Iterator<>(&m_Blocks, m_FrontIndex, m_FrontOffset);
        }
        constexpr Iterator<> end()
        {
            return Iterator<>(&m_Blocks, m_BackIndex, m_BackOffset);
        }

        constexpr ConstIterator begin() const
        {
            return ConstIterator(&m_Blocks, m_FrontIndex, m_FrontOffset);
        }
        constexpr ConstIterator end() const
        {
            return ConstIterator(&m_Blocks, m_BackIndex, m_BackOffset);
        }

        constexpr ReverseIterator rbegin() { return ReverseIterator(end()); }
        constexpr ReverseIterator rend() { return ReverseIterator(begin()); }

        constexpr ConstReverseIterator rbegin() const
        {
            return ConstReverseIterator(end());
        }
        constexpr ConstReverseIterator rend() const
        {
            return ConstReverseIterator(begin());
        }

        constexpr Iterator<> Erase(Iterator<> pos)
        {
            Vector<T> temp;
            for (auto it = begin(); it != end(); ++it)
                if (it != pos) temp.PushBack(*it);

            Clear();
            for (const auto& e : temp) PushBack(e);
            return begin() + (pos - begin());
        }

        constexpr void Clear()
        {
            m_Blocks.Clear();
            m_Blocks.PushBack(BlockRef::Create());
            m_FrontIndex  = 0;
            m_BackIndex   = 0;
            m_FrontOffset = BlockSize / 2;
            m_BackOffset  = BlockSize / 2;
        }
        constexpr usize Size() const
        {
            return (m_BackIndex - m_FrontIndex) * BlockSize
                 + (m_BackOffset - m_FrontOffset);
        }

        constexpr bool        Empty() const { return Size() == 0; }

        inline constexpr void Swap(Deque& other)
        {
            using Prism::Swap;

            Swap(m_Blocks, other.m_Blocks);
            Swap(m_FrontIndex, other.m_FrontIndex);
            Swap(m_BackIndex, other.m_BackIndex);

            Swap(m_FrontOffset, other.m_FrontOffset);
            Swap(m_BackOffset, other.m_BackOffset);
        }

      private:
        using BlockType                  = Block<T>;
        using BlockRef                   = Ref<BlockType>;

        static constexpr usize BlockSize = BlockType::BlockSize;

        Vector<BlockRef>       m_Blocks;
        usize                  m_FrontIndex  = 0;
        usize                  m_BackIndex   = 0;

        // Offsets for front and back (relative to their block)
        usize                  m_FrontOffset = BlockSize / 2;
        usize                  m_BackOffset  = BlockSize / 2;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Deque;
#endif
