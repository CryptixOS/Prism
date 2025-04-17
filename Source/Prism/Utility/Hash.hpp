/*
 * Created by v1tr10l7 on 16.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism::Hash
{
    inline usize MurmurHash2(const char* key, u64 length, u64 seed)
    {
        // Marmur Hash constants
        constexpr u64 MULTIPLIER   = 0xc6a4a7935bd1e995;
        constexpr i32 SHIFT_AMOUNT = 47;

        u64           hash         = seed ^ (length * MULTIPLIER);

        const u8*     chunks       = reinterpret_cast<const u8*>(key);
        const u64     chunkCount   = length / 8;

        for (u64 i = 0; i < chunkCount; ++i)
        {
            // NOTE(v1tr10l7): We use memcpy here to avoid unaligned accesses
            u64 chunk = 0;
            std::memcpy(&chunk, chunks + i * 8, sizeof(u64));

            chunk *= MULTIPLIER;
            chunk ^= chunk >> SHIFT_AMOUNT;
            chunk *= MULTIPLIER;

            hash ^= chunk;
            hash *= MULTIPLIER;
        }

        auto remaining = chunks + (chunkCount * 8);

        switch (length & 7)
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
}; // namespace Prism::Hash

#if PRISM_TARGET_CRYPTIX == 1
namespace Hash = Prism::Hash;
#endif
