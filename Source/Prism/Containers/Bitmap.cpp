/*
 * Created by v1tr10l7 on 14.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Bitmap.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism
{
    Bitmap::Bitmap(u8* data, const usize size, const u8 value)
    {
        Initialize(data, size, value);
    }

    void Bitmap::Initialize(u8* data, const usize size, const u8 value)
    {
        m_Data       = data;
        m_Size       = size;

        m_EntryCount = m_Size * 8;
        SetAll(value);
    }
    void Bitmap::Allocate(usize entryCount)
    {
        m_EntryCount = entryCount;

        m_Size       = Math::DivRoundUp(entryCount, 8);
        m_Data       = new u8[m_Size];
    }
    void Bitmap::Free()
    {
        delete[] m_Data;
        m_Data       = nullptr;
        m_EntryCount = 0;
        m_Size       = 0;
    }

    bool Bitmap::At(const usize index) const
    {
        const usize byte = index / 8;
        const usize bit  = index % 8;

        return m_Data[byte] & Bit(bit);
    }

    void Bitmap::SetAll(const u8 value) { Memory::Fill(m_Data, value, m_Size); }
    void Bitmap::SetIndex(const usize index, const bool value)
    {
        const usize byte = index / 8;
        const usize bit  = index % 8;
        assert(byte < m_Size);

        if (value) m_Data[byte] |= Bit(bit);
        else m_Data[byte] &= ~Bit(bit);
    }
}; // namespace Prism
