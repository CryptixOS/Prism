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

#define PM_ALWAYS_INLINE [[gnu::always_inline]]
#define PM_USED          [[gnu::used]]
#define PM_PACKED        [[gnu::packed]]

#if defined(__has_builtin)
    #if __has_builtin(__builtin_LINE)
        #define PM_LINE __builtin_LINE()
    #endif
    #if __has_builtin(__builtin_COLUMN)
        #define PM_COLUMN __builtin_COLUMN()
    #endif
    #if __has_builtin(__builtin_FILE)
        #define PM_FILENAME __builtin_FILE()
    #endif
    #if __has_builtin(__builtin_FUNCTION)
        #define PM_FUNCTION_NAME __builtin_FUNCTION()
    #endif
    #if __has_builtin(__builtin_frame_address)
        #define PrismGetFrameAddress(n) __builtin_frame_address(n)
    #endif
#endif

#ifndef PM_LINE
    #define PM_LINE 0
#endif

#ifndef PM_COLUMN
    #define PM_COLUMN 0
#endif
#ifndef PM_FILE
    #define PM_FILE 0
#endif
#ifndef PM_FUNCTION_NAME
    #define PM_FUNCTION_NAME 0
#endif
#ifndef PrismGetFrameAddress
    #define PrismGetFrameAddress 0
#endif
