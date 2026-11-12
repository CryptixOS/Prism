/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <utility>

#include <Prism/Core/Types.hpp>
import Prism.Test.Test;
import Prism.String.Formatter;
import Prism.String.String;
import Prism.String.StringBuilder;
import Prism.String.StringUtils;
import Prism.Containers.DoublyLinkedList;


using namespace Prism;

TEST_CASE(string_builder_format_to)
{
    StringBuilder<char> sb;

    FormatTo(sb, "val: {:08x}\n", 255);
    FormatTo(sb, "val: {:6}\n", 42);
    FormatTo(sb, "val: {:#X}\n", 0xABCDEF);
    FormatTo(sb, "val: {:#b}\n", 32);

    BasicString<char> final = sb;
    EXPECT_EQ(final, "val: 000000ff\nval:     42\nval: 0XABCDEF\nval: 0b100000\n"_s);
    EXPECT_EQ(sb.TotalLength(), final.Size());
}
