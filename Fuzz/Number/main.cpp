/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
// Number parsing and printing: ToNumber must accept any text without
// undefined behaviour, and ToString(ToNumber(x)) must parse back.
#include <Prism/Core/Types.hpp>
import Prism.String.StringUtils;

using namespace Prism;

template <typename T>
static void Check(StringView text, usize base)
{
    T     value = StringUtils::ToNumber<T>(text, base);
    char       digits[80];
    StringView printed = StringUtils::ToString(value, digits, 10);
    if (StringUtils::ToNumber<T>(printed, 10) != value)
        __builtin_trap();
}

extern "C" int LLVMFuzzerTestOneInput(const u8* data, usize size)
{
    if (size == 0) return 0;
    usize      base = 2 + data[0] % 35;
    StringView text(reinterpret_cast<const char*>(data + 1), size - 1);

    Check<i8>(text, base);
    Check<u8>(text, base);
    Check<i32>(text, base);
    Check<u32>(text, base);
    Check<i64>(text, base);
    Check<u64>(text, base);
    return 0;
}
