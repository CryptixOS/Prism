/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Span.hpp>
#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Memory/Pointer.hpp>

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
        constexpr Buffer(Buffer&& other)
            : m_Buffer(Move(other.m_Buffer))
        {
        }

        inline Buffer& operator=(const Buffer& other)
        {
            m_Buffer = other.m_Buffer;

            return *this;
        }
        inline Buffer& operator=(Buffer&& other)
        {
            m_Buffer = Move(other.m_Buffer);

            return *this;
        }

        void           Resize(usize size) { m_Buffer.Resize(size); }
        void           Reserve(usize capacity) { m_Buffer.Reserve(capacity); }
        void           ShrinkToFit() { m_Buffer.ShrinkToFit(); }

        Byte*          Raw() { return m_Buffer.Raw(); }
        const Byte*    Raw() const { return m_Buffer.Raw(); }

        constexpr bool Empty() const { return m_Buffer.Empty(); }
        usize          Size() const { return m_Buffer.Size(); }
        usize          Capacity() const { return m_Buffer.Capacity(); }

        Byte&          operator[](usize index) { return m_Buffer[index]; }

        void           Clear() { m_Buffer.Clear(); }

        template <PointerHolder T>
        void Read(usize offset, T dest, usize bytes)
        {
            assert(offset + bytes <= m_Buffer.Size());

            if constexpr (IsSameV<T, Pointer>)
                Memory::Copy(dest.Raw(), Raw() + offset, bytes);
            else Memory::Copy(dest, Raw() + offset, bytes);
        }
        void Write(usize offset, const Byte* src, usize bytes)
        {
            assert((offset + bytes) <= m_Buffer.Size());
            Memory::Copy(Raw() + offset, src, bytes);
        }

        void Fill(Byte value) { Memory::Fill(Raw(), value, Size()); }
        void Fill(usize offset, Byte value)
        {
            assert(offset < Size());

            Memory::Fill(Raw() + offset, value, Size() - offset);
        }

        class Prism::Span<Byte> Span()
        {
            return Prism::Span(m_Buffer.Raw(), m_Buffer.Size());
        }
        class Prism::Span<const Byte> Span() const
        {
            return Prism::Span(m_Buffer.Raw(), m_Buffer.Size());
        }

      private:
        Vector<Byte> m_Buffer;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX != 0
using Prism::Buffer;
using Prism::Byte;
#endif
