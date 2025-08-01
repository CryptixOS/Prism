/*
 * Created by v1tr10l7 on 03.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Memory/Endian.hpp>

#include <Prism/Memory/Buffer.hpp>
#include <Prism/Memory/Memory.hpp>

#include <Prism/Utility/Math.hpp>

namespace Prism
{
    template <Endian E = Endian::eNative>
    class ByteStream
    {
      public:
        ByteStream() = default;
        ByteStream(u8* data, usize size)
            : m_Data(data)
            , m_Size(size)
        {
        }
        ByteStream(Buffer& buffer)
            : m_Data(buffer.Raw())
            , m_Size(buffer.Size())
        {
        }

        constexpr u8*   begin() const { return m_Data; }
        constexpr u8*   end() const { return m_Data + m_Offset; }

        constexpr u8*   Raw() const { return m_Data; }
        constexpr usize Size() const { return m_Size; }
        constexpr usize Offset() const { return m_Offset; }
        constexpr       operator bool() const { return m_Offset < m_Size; }
        constexpr bool  IsEndOfStream() const { return m_Offset >= m_Size; }

        void            Load(u8* data, usize size)
        {
            m_Data = data;
            m_Size = size;
        }

        inline constexpr void Advance(usize offset) { m_Offset += offset; }
        inline constexpr void Seek(usize offset) { m_Offset = offset; }
        inline constexpr void Skip(usize size)
        {
            if (m_Offset + size > m_Size) m_Offset = m_Size;
            else m_Offset += size;
        }

        template <Integral T>
        T Read()
        {
            assert(m_Offset + sizeof(T) <= m_Size);

            T value;
            Memory::Copy(&value, m_Data + m_Offset, sizeof(T));
            m_Offset += sizeof(T);

            return ConvertEndian<E>(value);
        }
        template <typename T>
        void Read(T* dest, usize size)
        {
            assert(m_Offset + size <= m_Size);

            Memory::Copy(dest, m_Data + m_Offset, size);
            m_Offset += size;
        }

        void Write(ByteStream<E>& inStream, usize size)
        {
            assert(m_Offset + size <= m_Size);

            inStream.Read(m_Data + m_Offset, size);
            m_Offset += size;
        }
        template <typename T>
        void Write(T* data, usize size)
        {
            assert(m_Offset + size <= m_Size);

            Memory::Copy(m_Data + m_Offset, data, size);
            m_Offset += size;
        }
        template <typename T, usize Size>
        void operator>>(T (&dest)[Size])
        {
            assert(m_Offset + Size <= m_Size);

            Memory::Copy(reinterpret_cast<void*>(dest), m_Data + m_Offset,
                         Size);
            m_Offset += Size;
        }

        template <typename T>
        void operator>>(T& dest)
            requires(sizeof(T) > 8)
        {
            assert(m_Offset + sizeof(T) <= m_Size);

            Memory::Copy(&dest, m_Data + m_Offset, sizeof(T));
            m_Offset += sizeof(T);
        }
        template <typename T>
        void operator>>(T& dest)
            requires(sizeof(T) <= 8)
        {
            assert(m_Offset + sizeof(T) <= m_Size);

            T value;
            Memory::Copy(&value, m_Data + m_Offset, sizeof(T));

            dest = ToEndian<E>(value);
            m_Offset += sizeof(T);
        }

      private:
        u8*   m_Data   = nullptr;
        usize m_Size   = 0;
        usize m_Offset = 0;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::ByteStream;
#endif
