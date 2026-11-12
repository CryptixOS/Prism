/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Compiler.hpp>

namespace Prism
{
};

#ifndef PRISM_TARGET_CRYPTIX
    #define PRISM_TARGET_CRYPTIX 0
#endif

/**
 * PRISM_STD_INTEROP lets Prism's iterator and range concepts understand the
 * standard library's iterators and ranges (std iterator tags,
 * std::ranges::enable_borrowed_range / enable_view), and makes Prism views
 * std views too. It's on by default for hosted builds that have <iterator>
 * and <ranges>, off for CryptixOS. Define it to 0 or 1 to override.
 */
#ifndef PRISM_STD_INTEROP
    #if PRISM_TARGET_CRYPTIX == 0 && PrismHasInclude(<iterator>)               \
        && PrismHasInclude(<ranges>)
        #define PRISM_STD_INTEROP 1
    #else
        #define PRISM_STD_INTEROP 0
    #endif
#endif
