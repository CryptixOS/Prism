/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
// Format strings from the fuzzer, with a fixed set of arguments. Prism::Format
// must never crash, read out of bounds or hang on a bad format string; it
// reports the error and keeps going.
#include <Prism/Core/Types.hpp>
import Prism.String.Formatter;

using namespace Prism;

extern "C" int LLVMFuzzerTestOneInput(const u8* data, usize size)
{
    StringView format(reinterpret_cast<const char*>(data), size);
    // Widths and precisions are honoured up to INT_MAX, like the patched fmt:
    // correct, but padding gigabytes only makes the fuzzer time out. So keep
    // the integer arguments (which double as dynamic widths, "{:{6}}") small
    // and skip literal numbers of six digits or more.
    usize digits = 0;
    for (usize i = 0; i < size; ++i)
    {
        digits = (data[i] >= '0' && data[i] <= '9') ? digits + 1 : 0;
        if (digits >= 6) return 0;
    }

    String     out = Format(Runtime(format), 42, -7, "text", 'c', true,
                            static_cast<const void*>(data), 0xbeefULL,
                            Arg("name", 5), StringView("view"));
    (void)out;

    char buffer[64];
    (void)FormatToN(buffer, sizeof(buffer), Runtime(format), 1, 2, 3, "four");
    (void)FormattedSize(Runtime(format), -1, "x");
    return 0;
}
