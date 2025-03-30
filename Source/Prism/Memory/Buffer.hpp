/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Types.hpp>

#include <span>

namespace Prism
{
    using Byte = u8;
    class Buffer
    {
      public:
        constexpr Buffer() = default;
        explicit Buffer(usize size)
            : m_Buffer(size)
        {
        }

        void        Resize(usize size) { m_Buffer.Resize(size); }
        void        Reserve(usize capacity) { m_Buffer.Reserve(capacity); }

        Byte*       Raw() { return m_Buffer.Raw(); }
        const Byte* Raw() const { return m_Buffer.Raw(); }
        usize       Size() const { return m_Buffer.Size(); }
        usize       Capacity() const { return m_Buffer.Capacity(); }

        Byte&       operator[](usize index)
        {
            assert(index < m_Buffer.Size());
            return m_Buffer[index];
        }

        void Read(usize offset, Byte* dest, usize bytes)
        {
            assert(offset + bytes <= m_Buffer.Size());
            std::memcpy(dest, Raw() + offset, bytes);
        }
        void Write(usize offset, const Byte* src, usize bytes)
        {
            assert(offset + bytes <= m_Buffer.Size());
            std::memcpy(Raw() + offset, src, bytes);
        }

        void Fill(Byte value) { std::memset(Raw(), value, Size()); }
        void Fill(usize offset, Byte value)
        {
            assert(offset < Size());

            std::memset(Raw() + offset, value, Size() - offset);
        }

        std::span<Byte>       Span() { return std::span<Byte>(m_Buffer); }
        std::span<const Byte> Span() const
        {
            return std::span<const Byte>(m_Buffer);
        }

      private:
        Vector<Byte> m_Buffer;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::Buffer;
using Prism::Byte;
#endif
