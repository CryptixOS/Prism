/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Limits.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Memory/Endian.hpp>
#include <Prism/Memory/Pointer.hpp>

namespace Prism
{
    // TODO(v1tr10l7): Handle BigEndian reads
    template <Endian E = Endian::eNative>
    class BitStream
    {
      public:
        BitStream() = default;
        BitStream(u8* data, usize size)
            : m_CurrentPointer(data)
            , m_StreamStart(data)
            , m_StreamEnd(data + size)
            , m_BitBuffer(0)
            , m_BitCount(0)
        {
        }

        inline bool IsEndOfStream() const
        {
            return m_CurrentPointer >= m_StreamEnd;
        }

        inline void Seek(usize offset)
        {
            m_CurrentPointer = m_StreamStart + offset;
        }
        inline usize Tell() const { return m_StreamEnd - m_CurrentPointer; }

        template <std::integral T>
        inline T Read()
        {
            assert(m_CurrentPointer + sizeof(T) <= m_StreamEnd);

            T value;
            std::memcpy(&value, m_CurrentPointer, sizeof(T));
            m_CurrentPointer += sizeof(T);

            return ConvertEndian<E>(value);
        }
        template <typename T>
        inline usize Read(T* dest, usize count)
        {
            assert(m_CurrentPointer + count <= m_StreamEnd);
            AlignToByteBoundary();

            u8*   destBuffer = reinterpret_cast<u8*>(dest);
            usize bytesRead  = 0;
            if (m_BitCount > 0)
            {
                auto bitsToRead = std::min(count * 8, m_BitCount);
                auto result     = ReadBits(bitsToRead);

                bytesRead       = bitsToRead / 8;
                std::memcpy(destBuffer, &result, bytesRead);
                destBuffer += bytesRead;
            }

            std::memcpy(destBuffer, m_CurrentPointer, count - bytesRead);
            m_CurrentPointer += count - bytesRead;

            return count;
        }

        inline bool ReadBit() { return ReadBits(1); }
        template <std::integral T = u64>
        inline T ReadBits(usize count)
            requires(sizeof(T) <= sizeof(u64))
        {
            auto result = PeekBits<T>(count);
            DropBits(count);

            return result;
        }
        template <std::integral T = u64>
        inline T PeekBits(usize count)
            requires(sizeof(T) <= 8)
        {
            if (count > m_BitCount) FillBits();

            constexpr auto MAX_VALUE = NumericLimits<T>::Max();
            constexpr auto DIGITS    = NumericLimits<T>::Digits;
            count                    = std::min(count, m_BitCount);

            return m_BitBuffer & (MAX_VALUE >> (DIGITS - count));
        }

        inline void DropBits(usize count)
        {
            if (count > m_BitCount) count = m_BitCount;

            m_BitBuffer >>= count;
            m_BitCount -= count;
        }

        inline u8 AlignToByteBoundary()
        {
            u8 remainingBits = 0;
            if (auto offset = m_BitCount % 8; offset != 0)
            {
                remainingBits = m_BitBuffer & (Bit(offset) - 1);
                DropBits(offset);
            }

            return remainingBits;
        }

        inline void FillBits()
        {
            do {
                if (m_BitBuffer >= (1u << m_BitCount))
                {
                    m_CurrentPointer = m_StreamEnd;
                    return;
                }

                m_BitBuffer |= static_cast<u32>(Read<u8>()) << m_BitCount;
                m_BitCount += 8;
            } while (m_BitCount <= 24);
        }

      private:
        u8*   m_CurrentPointer = nullptr;
        u8*   m_StreamStart    = nullptr;
        u8*   m_StreamEnd      = nullptr;

        u32   m_BitBuffer      = 0;
        usize m_BitCount       = 0;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::BitStream;
#endif
