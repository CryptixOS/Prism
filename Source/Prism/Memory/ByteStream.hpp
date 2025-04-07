/*
 * Created by v1tr10l7 on 03.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Endian.hpp>

#include <concepts>

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

        constexpr u8*   begin() const { return m_Data; }
        constexpr u8*   end() const { return m_Data + m_Offset; }

        constexpr u8*   Raw() const { return m_Data; }
        constexpr usize Size() const { return m_Size; }
        constexpr usize Offset() const { return m_Offset; }
        constexpr       operator bool() const { return m_Offset < m_Size; }

        void            Load(u8* data, usize size)
        {
            m_Data = data;
            m_Size = size;
        }

        inline constexpr void Seek(usize offset) { m_Offset = offset; }
        inline constexpr void Skip(usize size)
        {
            if (m_Offset + size > m_Size) m_Offset = m_Size;
            else m_Offset += size;
        }

        template <std::integral T>
        T Read()
        {
            assert(m_Offset + sizeof(T) <= m_Size);

            T value;
            std::memcpy(&value, m_Data + m_Offset, sizeof(T));
            m_Offset += sizeof(T);

            return ConvertEndian<E>(value);
        }
        template <typename T>
        void Read(T* dest, usize size)
        {
            assert(m_Offset + size <= m_Size);

            std::memcpy(dest, m_Data + m_Offset, size);
            m_Offset += size;
        }

        template <typename T>
        void Write(T* data, usize size)
        {
            assert(m_Offset + size <= m_Size);

            std::memcpy(m_Data + m_Offset, data, size);
            m_Offset += size;
        }
        template <typename T, usize Size>
        void operator<<(T (&dest)[Size])
        {
            assert(m_Offset + Size <= m_Size);

            std::memcpy(reinterpret_cast<void*>(dest), m_Data + m_Offset, Size);
            m_Offset += Size;
        }

        template <typename T>
        void operator<<(T& dest)
            requires(sizeof(T) > 8)
        {
            assert(m_Offset + sizeof(T) <= m_Size);

            std::memcpy(&dest, m_Data + m_Offset, sizeof(T));
            m_Offset += sizeof(T);
        }
        template <typename T>
        void operator<<(T& dest)
            requires(sizeof(T) <= 8)
        {
            assert(m_Offset + sizeof(T) <= m_Size);

            T value;
            std::memcpy(&value, m_Data + m_Offset, sizeof(T));

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
