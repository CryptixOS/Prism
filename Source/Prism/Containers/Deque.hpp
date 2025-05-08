/*
 * Created by v1tr10l7 on 30.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

#include <cassert>

namespace Prism
{
    template <typename T>
    class Deque
    {
      public:
        Deque()
        {
            m_MapSize    = 8;
            m_Map        = new BlockPointer[m_MapSize];

            usize mid    = m_MapSize / 2;
            m_StartBlock = m_EndBlock = mid;
            m_StartOffset = m_EndOffset = 0;
            m_Map[mid]                  = new T[BLOCK_SIZE];
        }
        ~Deque()
        {
            for (usize i = m_StartBlock; i <= m_EndBlock; ++i)
                delete[] m_Map[i];
            delete[] m_Map;
        }

        constexpr usize Size() const { return m_Size; }
        constexpr bool  Empty() const { return Size() == 0; }

        constexpr T&    operator[](usize index)
        {
            assert(index < Size());
            usize logical = m_StartBlock * BLOCK_SIZE + m_StartOffset + index;
            usize block   = logical / BLOCK_SIZE;
            usize offset  = logical % BLOCK_SIZE;

            return m_Map[block][offset];
        }
        constexpr const T& operator[](usize index) const
        {
            assert(index < Size());
            usize logical = m_StartBlock * BLOCK_SIZE + m_StartOffset + index;
            usize block   = logical / BLOCK_SIZE;
            usize offset  = logical % BLOCK_SIZE;

            return m_Map[block][offset];
        }

        void PushBack(const T& value)
        {
            if (m_EndOffset == BLOCK_SIZE)
            {
                ++m_EndBlock;
                m_EndOffset = 0;
                EnsureCapacityBack();

                assert(m_EndBlock < m_MapSize);
                m_Map[m_EndBlock] = new T[BLOCK_SIZE];
            }

            m_Map[m_EndBlock][m_EndOffset++] = value;
            ++m_Size;
        }
        void PushFront(const T& value)
        {
            if (m_StartOffset == 0)
            {
                EnsureCapacityFront();
                --m_StartBlock;
                m_StartOffset       = BLOCK_SIZE;

                m_Map[m_StartBlock] = new T[BLOCK_SIZE];
            }

            m_Map[m_StartBlock][--m_StartOffset] = value;
            ++m_Size;
        }
        void PopBack()
        {
            assert(!Empty());
            if (m_EndOffset == 0)
            {
                delete[] m_Map[m_EndBlock];
                --m_EndBlock;
                m_EndOffset = BLOCK_SIZE;
            }

            --m_EndOffset;
            --m_Size;
        }
        void PopFront()
        {
            assert(!Empty());
            ++m_StartOffset;
            --m_Size;

            if (m_StartOffset == BLOCK_SIZE)
            {
                delete[] m_Map[m_StartBlock];
                ++m_StartBlock;
                m_StartOffset = 0;
            }
        }

      private:
        constexpr static usize BLOCK_SIZE = 64;
        using Block                       = T[BLOCK_SIZE];
        using BlockPointer                = T*;

        BlockPointer* m_Map               = nullptr;
        usize         m_MapSize           = 0;

        usize         m_StartBlock        = 0;
        usize         m_StartOffset       = 0;
        usize         m_EndBlock          = 0;
        usize         m_EndOffset         = 0;
        usize         m_Size              = 0;

        void          EnsureCapacityBack()
        {
            if (m_EndBlock + 1 >= m_MapSize) GrowMap();
        }
        void EnsureCapacityFront()
        {
            if (m_StartBlock == 0) GrowMap();
        }
        void GrowMap()
        {
            usize         newSize = m_MapSize * 2;
            BlockPointer* newMap  = new BlockPointer[newSize]{};

            usize         offset  = (newSize - m_MapSize) / 2;
            for (usize i = 0; i < m_MapSize; ++i) newMap[i + offset] = m_Map[i];

            delete[] m_Map;
            m_Map = newMap;
            m_StartBlock += offset;
            m_EndBlock += offset;
            m_MapSize = newSize;
        }
    };
}; // namespace Prism
