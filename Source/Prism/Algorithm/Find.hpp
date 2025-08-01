/*
 * Created by v1tr10l7 on 31.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    template <typename Iterator, typename UnaryPredicate>
    [[nodiscard]] constexpr Iterator FindIf(Iterator first, Iterator last,
                                            UnaryPredicate&& pred)
    {
        for (; first != last; ++first)
            if (pred(*first)) return first;
        return last;
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::FindIf;
#endif
