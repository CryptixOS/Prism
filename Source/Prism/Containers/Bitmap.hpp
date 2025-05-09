/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Utility/Math.hpp>

#include <cstring>

namespace Prism
{
    class Bitmap
    {
      public:
        Bitmap() = default;
        Bitmap(u8* data, const usize size, const u8 value = 1)
        {
            Initialize(data, size, value);
        }

        inline void Initialize(u8* data, const usize size, const u8 value = 1)
        {
            m_Data       = data;
            m_Size       = size;

            m_EntryCount = m_Size * 8;
            SetAll(value);
        }
        inline void Allocate(usize entryCount)
        {
            m_EntryCount = entryCount;

            m_Size       = Math::DivRoundUp(entryCount, 8);
            m_Data       = new u8[m_Size];
        }
        inline void Free()
        {
            delete[] m_Data;
            m_Data       = nullptr;
            m_EntryCount = 0;
            m_Size       = 0;
        }

        constexpr u8*   Raw() const { return m_Data; }
        constexpr usize GetSize() const { return m_EntryCount; }

        inline void     SetAll(const u8 value)
        {
            std::memset(m_Data, value, m_Size);
        }
        inline void SetIndex(const usize index, const bool value)
        {
            const usize byte = index / 8;
            const usize bit  = index % 8;
            assert(byte < m_Size);

            if (value) m_Data[byte] |= Bit(bit);
            else m_Data[byte] &= ~Bit(bit);
        }
        inline bool GetIndex(const usize index) const
        {
            const usize byte = index / 8;
            const usize bit  = index % 8;

            return m_Data[byte] & Bit(bit);
        }

      private:
        u8*   m_Data;
        usize m_EntryCount = 0;
        usize m_Size       = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Bitmap;
#endif
