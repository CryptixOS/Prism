/*
 * Created by v1tr10l7 on 12.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Algorithm/FNV1aHash.hpp>
#include <Prism/Containers/Span.hpp>
#include <Prism/Core/Integer128.hpp>
#include <Prism/Core/Limits.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    namespace Murmur
    {
        usize Hash2(u8* input, usize length, const u64 seed);
        usize Hash2(Span<u8> input, const u64 seed);
        u128  Hash3(Span<u8> input, const u32 seed);
    }; // namespace Murmur
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
namespace Murmur = Prism::Murmur;
#endif
