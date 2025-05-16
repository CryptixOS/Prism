/*
 * Created by v1tr10l7 on 09.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Assertions.hpp>

namespace Prism
{
#if !defined(PRISM_TARGET_CRYPTIX) || PRISM_TARGET_CRYPTIX == 0
    void earlyPanic(StringView msg)
    {
        PrismFatal("Panic! =>\n{}", msg);
        __builtin_trap();
    }
    [[noreturn]]
    void panic(StringView msg)
    {
        PrismFatal("Panic! =>\n{}", msg);
        __builtin_trap();
    }
#endif
}; // namespace Prism
