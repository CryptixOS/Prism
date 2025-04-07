/*
 * Created by v1tr10l7 on 26.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#ifndef PM_DISABLE_NOEXCEPT
    #define PM_NOEXCEPT noexcept
#else
    #define PM_NOEXCEPT
#endif

#define PM_ALWAYS_INLINE           [[gnu::always_inline]]
#define PM_USED [[gnu::used]]
#define PM_PACKED [[gnu::packed]]

#define PM_LINE                 __builtin_LINE()
#define PM_COLUMN               __builtin_COLUMN()
#define PM_FILENAME             __builtin_FILE()
#define PM_FUNCTION_NAME        __builtin_FUNCTION()

#define PrismGetFrameAddress(n) __builtin_frame_address(n)
