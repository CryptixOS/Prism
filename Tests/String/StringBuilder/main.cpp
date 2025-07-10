/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <utility>

#include <Prism/Containers/DoublyLinkedList.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/String/Formatter.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringBuilder.hpp>
#include <Prism/String/StringUtils.hpp>

#include <Prism/Core/Types.hpp>

using namespace Prism;

int main()
{
    StringBuilder<char> sb;

    FormatTo(sb, "val: {:08x}\n", 255);     // "val: 000000ff"
    FormatTo(sb, "val: {:6}\n", 42);        // "val:     42"
    FormatTo(sb, "val: {:#X}\n", 0xABCDEF); // "val: ABCDEF"
    FormatTo(sb, "val: {:#b}\n", 32);       // "val: ABCDEF"
    //
    BasicString<char> final = sb;
    printf("final: %s\n", final.Raw());

    return 0;
}
