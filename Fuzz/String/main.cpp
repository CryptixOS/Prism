/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
// String and StringView searches with fuzzer haystacks and needles: the first
// byte splits the input into the two.
#include <Prism/Core/Types.hpp>
import Prism.String.String;
import Prism.String.StringView;

using namespace Prism;

extern "C" int LLVMFuzzerTestOneInput(const u8* data, usize size)
{
    if (size == 0) return 0;
    usize      split    = data[0] % size;
    const char* bytes   = reinterpret_cast<const char*>(data + 1);
    StringView haystack(bytes, split);
    StringView needle(bytes + split, size - 1 - split);

    usize      found = haystack.Find(needle);
    if (found != StringView::NPos
        && (found + needle.Size() > haystack.Size()
            || haystack.Substr(found, needle.Size()) != needle))
        __builtin_trap();
    (void)haystack.RFind(needle);
    (void)haystack.StartsWith(needle);
    (void)haystack.EndsWith(needle);
    (void)haystack.Contains(needle);

    String owned(haystack);
    owned += needle;
    if (owned.Size() != haystack.Size() + needle.Size()) __builtin_trap();
    for (const String& part : owned.Split(':')) (void)part.Size();
    return 0;
}
