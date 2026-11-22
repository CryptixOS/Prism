/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

// No includes: Compiler.hpp, and so every Prism header, starts with this.

namespace Prism
{
};

// Every configuration macro has a default here, so `#if PRISM_FOO` never
// sees an undefined macro (-Wundef). Documentation/Configuration.md lists
// them all.

#ifndef PRISM_TARGET_CRYPTIX
    #define PRISM_TARGET_CRYPTIX 0
#endif
#ifndef PRISM_TARGET_CARBONC
    #define PRISM_TARGET_CARBONC 0
#endif
#ifndef PRISM_USE_NAMESPACE
    #define PRISM_USE_NAMESPACE 0
#endif
#ifndef PRISM_PREFIXLESS_MACROS
    #define PRISM_PREFIXLESS_MACROS 0
#endif
#ifndef PRISM_LOG_ENABLE
    #define PRISM_LOG_ENABLE 1
#endif

/**
 * PRISM_HARDENED=1 turns the containers', views' and strings' precondition
 * checks (bounds, non-empty, valid iterators) and their size-overflow checks
 * into PrismCheck()s that stay on in release builds; a failed one traps.
 * Without it they are assert()s.
 */
#ifndef PRISM_HARDENED
    #define PRISM_HARDENED 0
#endif

/**
 * PRISM_STD_INTEROP lets Prism's iterator and range concepts understand the
 * standard library's iterators and ranges (std iterator tags,
 * std::ranges::enable_borrowed_range / enable_view), and makes Prism views
 * std views too. It's on by default for hosted builds that have <iterator>
 * and <ranges>, off for CryptixOS. Define it to 0 or 1 to override.
 */
#ifndef PRISM_STD_INTEROP
    #if PRISM_TARGET_CRYPTIX == 0 && __has_include(<iterator>)                \
        && __has_include(<ranges>)
        #define PRISM_STD_INTEROP 1
    #else
        #define PRISM_STD_INTEROP 0
    #endif
#endif
