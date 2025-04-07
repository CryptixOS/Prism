/*
 * Created by v1tr10l7 on 07.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Memory/ByteStream.hpp>

namespace Prism
{
    namespace CRC32
    {
        constexpr u32 CRC32_POLY = 0xedb88320;

        constexpr u32 GenerateTableEntry(i32 i)
        {
            u32 crc = i;
            for (i32 j = 0; j < 8; ++j)
                crc = (crc >> 1) ^ ((crc & 1) ? CRC32_POLY : 0);
            return crc;
        }

        constexpr u32 DoChecksum(const u8* data, usize length)
        {
            static Array<u32, 256> table       = {};
            static bool            initialized = false;

            if (!initialized)
            {
                for (i32 i = 0; i < 256; ++i) table[i] = GenerateTableEntry(i);
                initialized = true;
            }

            u32 crc = 0xffffffff;
            for (usize i = 0; i < length; ++i)
                crc = (crc >> 8) ^ table[(crc ^ data[i]) & 0xFF];

            return ~crc;
        }
    }; // namespace CRC32
    namespace Adler32
    {
        constexpr u32 MOD_ADLER = 65521;

        constexpr u32 DoChecksum(const u8* data, usize len)
        {
            u32 a = 1, b = 0;
            for (usize i = 0; i < len; ++i)
            {
                a = (a + data[i]) % MOD_ADLER;
                b = (b + a) % MOD_ADLER;
            }
            return (b << 16) | a;
        }
    }; // namespace Adler32
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
namespace CRC32   = Prism::CRC32;
namespace Adler32 = Prism::Adler32;
#endif
