/*
 * Created by v1tr10l7 on 24.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    i32 Printf(const char* format, ...);
    i32 SPrintf(char* string, const char* format, ...);
    i32 SnPrintf(char* string, usize size, const char* format, ...);
    i32 VPrintf(const char* format, va_list args);
    i32 VsPrintf(char* string, const char* format, va_list args);
    i32 VsnPrintf(char* string, usize size, const char* format, va_list args);
}; // namespace Prism
