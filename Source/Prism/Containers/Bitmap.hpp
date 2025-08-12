/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Assertions.hpp>

namespace Prism
{
    class Bitmap
    {
      public:
        class BitReference
        {
          public:
            constexpr BitReference(u8& byte, u8 mask)
                : m_Byte(byte)
                , m_Mask(mask)
            {
            }

            inline constexpr operator bool() const { return m_Byte & m_Mask; }
            inline constexpr BitReference& operator=(bool value)
            {
                if (value) m_Byte |= m_Mask;
                else m_Byte &= ~m_Mask;

                return *this;
            }
            inline constexpr BitReference& operator=(const BitReference& other)
            {
                return *this = static_cast<bool>(other);
            }

          private:
            u8& m_Byte;
            u8  m_Mask;
        };

        Bitmap() = default;
        Bitmap(u8* data, const usize size, const u8 value = 1);

        constexpr u8*                 Raw() const { return m_Data; }
        constexpr usize               BitCount() const { return m_EntryCount; }
        constexpr usize               ByteCount() const { return m_Size; }

        inline constexpr BitReference operator[](usize index)
        {
            usize byteIndex = index / 8;
            u8    mask      = static_cast<u8>(1u << (index % 8));

            return BitReference(m_Data[byteIndex], mask);
        }
        inline constexpr bool operator[](usize index) const
        {
            usize byteIndex = index / 8;
            usize mask      = static_cast<u8>(1u << (index % 8));

            return m_Data[byteIndex] & mask;
        }

        bool  At(const usize index) const;
        isize FindFirstSet(isize start = 0, isize end = -1)
        {
            for (isize i = start; i < end; i++)
                if (At(i)) return i;

            return -1;
        }
        isize FindFirstNotSet(isize start = 0, isize end = -1)
        {
            for (isize i = start; i < end; i++)
                if (!At(i)) return i;

            return -1;
        }

        void Initialize(u8* data, const usize size, const u8 value = 1);
        void Allocate(usize entryCount);
        void Free();

        void SetAll(const u8 value);
        void SetIndex(const usize index, const bool value);

      private:
        u8*   m_Data;
        usize m_EntryCount = 0;
        usize m_Size       = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Bitmap;
#endif
