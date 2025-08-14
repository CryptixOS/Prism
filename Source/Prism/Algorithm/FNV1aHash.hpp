/*
 * Created by v1tr10l7 on 14.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    namespace FNV1a
    {
        // Starting value for the FNV1a hashing algorithm
        constexpr usize OFFSET_BASIC = 0xcbf2'9ce4'8422'2325zu;
        // The prime used to concatenate byte with a hash
        constexpr usize PRIME        = 0x0100'0000'01b3zu;

        constexpr usize ConcatenateHash(u64 base, u8* input, usize length)
        {
            usize result = base;
            for (; length > 0; ++input, --length)
            {
                result ^= *input;
                result *= PRIME;
            }

            return result;
        }
        consteval usize CEHash(u8* input, usize length)
        {
            return ConcatenateHash(OFFSET_BASIC, input, length);
        }
        constexpr usize Hash(u8* input, usize length)
        {
            return ConcatenateHash(OFFSET_BASIC, input, length);
        }
    }; // namespace FNV1a
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
namespace FNV1a = Prism::FNV1a;
#endif
