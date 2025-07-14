/*
 * Created by v1tr10l7 on 12.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Algorithm/Hash.hpp>

namespace Prism::Murmur
{
    usize Hash2(Span<u8> input, const u64 seed)
    {
        // Marmur Hash constants
        constexpr u64 MULTIPLIER   = 0xc6a4'a793'5bd1'e995zu;
        constexpr i32 SHIFT_AMOUNT = 47;

        u64       hash = seed ^ (static_cast<u64>(input.Size()) * MULTIPLIER);

        const u64 chunkCount = input.Size() >> 3;

        for (u64 i = 0; i < chunkCount; ++i)
        {
            // NOTE(v1tr10l7): We use memcpy here to avoid unaligned accesses
            u64 chunk = 0;
            Memory::Copy(&chunk, input.Raw() + (i << 3), sizeof(u64));

            chunk *= MULTIPLIER;
            chunk ^= chunk >> SHIFT_AMOUNT;
            chunk *= MULTIPLIER;

            hash ^= chunk;
            hash *= MULTIPLIER;
        }

        auto remaining = input.Raw() + (chunkCount << 3);

        switch (input.Size() & 7)
        {
            case 7:
                hash ^= static_cast<u64>(remaining[6]) << 48;
                [[fallthrough]];
            case 6:
                hash ^= static_cast<u64>(remaining[5]) << 40;
                [[fallthrough]];
            case 5:
                hash ^= static_cast<u64>(remaining[4]) << 32;
                [[fallthrough]];
            case 4:
                hash ^= static_cast<u64>(remaining[3]) << 24;
                [[fallthrough]];
            case 3:
                hash ^= static_cast<u64>(remaining[2]) << 16;
                [[fallthrough]];
            case 2:
                hash ^= static_cast<u64>(remaining[1]) << 8;
                [[fallthrough]];
            case 1: hash ^= static_cast<u64>(remaining[0]); hash *= MULTIPLIER;
        };

        hash ^= hash >> SHIFT_AMOUNT;
        hash *= MULTIPLIER;
        hash ^= hash >> SHIFT_AMOUNT;

        return hash;
    }

    inline u64 RotateLeft(u64 x, i8 r) { return (x << r) | (x >> (64 - r)); }
    PM_ALWAYS_INLINE inline u64 Combine(u64 k)
    {
        k ^= k >> 33;
        k *= 0xff51'afd7'ed55'8ccdzu;
        k ^= k >> 33;
        k *= 0xc4ce'b9fe'1a85'ec53zu;
        k ^= k >> 33;

        return k;
    }

    // TODO(v1tr10l7): Endiannes
    u128 Hash3(Span<u8> input, const u32 seed)
    {
        const isize blockCount = input.Size() >> 4;

        u64         h1         = seed;
        u64         h2         = seed;

        const u64   c1         = 0x87c3'7b91'1142'53d5zu;
        const u64   c2         = 0x4cf5'ad43'2745'937fzu;

        for (isize i = 0; i < blockCount; i++)
        {
            u64 k1;
            u64 k2;
            Memory::Copy(&k1, input.Raw() + (i << 4), sizeof(u64));
            Memory::Copy(&k2, input.Raw() + (i << 4) + 8, sizeof(u64));

            k1 *= c1;
            k1 = RotateLeft(k1, 31);
            k1 *= c2;
            h1 ^= k1;

            h1 = RotateLeft(h1, 27);
            h1 += h2;
            h1 = h1 * 5 + 0x52dc'e729;

            k2 *= c2;
            k2 = RotateLeft(k2, 33);
            k2 *= c1;
            h2 ^= k2;

            h2 = RotateLeft(h2, 31);
            h2 += h1;
            h2 = h2 * 5 + 0x3849'5ab5;
        }

        const u8* tail
            = reinterpret_cast<const u8*>(input.Raw() + (blockCount << 4));

        u64 k1 = 0;
        u64 k2 = 0;

        switch (input.Size() & 15)
        {
            case 15: k2 ^= ((u64)tail[14]) << 48;
            case 14: k2 ^= ((u64)tail[13]) << 40;
            case 13: k2 ^= ((u64)tail[12]) << 32;
            case 12: k2 ^= ((u64)tail[11]) << 24;
            case 11: k2 ^= ((u64)tail[10]) << 16;
            case 10: k2 ^= ((u64)tail[9]) << 8;
            case 9:
                k2 ^= ((u64)tail[8]) << 0;
                k2 *= c2;
                k2 = RotateLeft(k2, 33);
                k2 *= c1;
                h2 ^= k2;

            case 8: k1 ^= ((u64)tail[7]) << 56;
            case 7: k1 ^= ((u64)tail[6]) << 48;
            case 6: k1 ^= ((u64)tail[5]) << 40;
            case 5: k1 ^= ((u64)tail[4]) << 32;
            case 4: k1 ^= ((u64)tail[3]) << 24;
            case 3: k1 ^= ((u64)tail[2]) << 16;
            case 2: k1 ^= ((u64)tail[1]) << 8;
            case 1:
                k1 ^= static_cast<u64>(tail[0]) << 0;
                k1 *= c1;
                k1 = RotateLeft(k1, 31);
                k1 *= c2;
                h1 ^= k1;
        };

        h1 ^= input.Size();
        h2 ^= input.Size();

        h1 += h2;
        h2 += h1;

        h1 = Combine(h1);
        h2 = Combine(h2);

        h1 += h2;
        h2 += h1;

        return u128(h2, h1);
    }
}; // namespace Prism::Murmur
