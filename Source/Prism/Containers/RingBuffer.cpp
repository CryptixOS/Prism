/*
 * Created by v1tr10l7 on 31.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/RingBuffer.hpp>

namespace Prism
{
    RingBuffer::RingBuffer(usize capacity)
        : m_Buffer(new u8[capacity])
        , m_Capacity(capacity)
    {
    }
    RingBuffer::~RingBuffer() { delete[] m_Buffer; }

    void RingBuffer::Reserve(usize newCapacity)
    {
        if (m_Capacity != 0) delete[] m_Buffer;
        m_Buffer   = new u8[newCapacity];
        m_Capacity = newCapacity;
    }

    usize RingBuffer::Read(u8* const buffer, usize count)
    {
        if (Used() == 0 || !buffer) return 0;
        count            = std::min(count, Used());

        auto  head       = m_Head.Load();
        auto  headMod    = head % Capacity();

        usize firstPart  = std::min(count, Capacity() - headMod);
        usize secondPart = count - firstPart;

        std::memcpy(buffer, m_Buffer + headMod, firstPart);
        if (secondPart) std::memcpy(buffer + firstPart, m_Buffer, secondPart);

        m_Head.Store((head + count) % (m_Capacity * 2));
        return static_cast<isize>(count);
    }
    usize RingBuffer::Write(const u8* const buffer, usize count)
    {
        usize free = Free();
        if (free == 0 || !buffer) return 0;

        count            = std::min(count, free);

        auto  tail       = m_Tail.Load();
        auto  tailMod    = tail % m_Capacity;

        usize firstPart  = std::min(count, m_Capacity - tailMod);
        usize secondPart = count - firstPart;

        std::memcpy(m_Buffer + tailMod, buffer, firstPart);
        if (secondPart > 0)
            std::memcpy(m_Buffer, buffer + firstPart, secondPart);

        m_Tail.Store((tail + count) % (m_Capacity * 2));
        return static_cast<isize>(count);
    }
}; // namespace Prism
