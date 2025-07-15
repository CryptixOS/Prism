/*
 * Created by v1tr10l7 on 08.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Memory/Memory.hpp>

#include <assert.h>
#include <stdio.h>  // for debugging if needed
#include <string.h> // for memcmp in validation fallback

using namespace Prism;

void TestSet()
{
    u8 buffer[16];
    Memory::Fill(buffer, 0xab, sizeof(buffer));

    for (u8 byte : buffer) assert(byte == 0xab);
}

void TestCopy()
{
    u8 src[16], dest[16];
    for (usize i = 0; i < 16; ++i) src[i] = static_cast<u8>(i);

    Memory::Copy(dest, src, 16);
    for (usize i = 0; i < 16; ++i) assert(dest[i] == src[i]);
}

void TestMoveOverlapForward()
{
    u8 buffer[32];
    for (usize i = 0; i < 32; ++i) buffer[i] = static_cast<u8>(i);

    Memory::Move(buffer + 4, buffer, 16); // overlap forward

    for (usize i = 0; i < 16; ++i) assert(buffer[i + 4] == i);
}

void TestMoveOverlapBackward()
{
    u8 buffer[32];
    for (usize i = 0; i < 32; ++i) buffer[i] = static_cast<u8>(i);

    Memory::Move(buffer, buffer + 4, 16); // overlap backward

    for (usize i = 0; i < 16; ++i) assert(buffer[i] == i + 4);
}

void TestCompare()
{
    u8 a[] = {1, 2, 3, 4};
    u8 b[] = {1, 2, 3, 4};
    u8 c[] = {1, 2, 3, 5};

    assert(Memory::Compare(a, b, 4) == 0);
    assert(Memory::Compare(a, c, 4) < 0);
    assert(Memory::Compare(c, a, 4) > 0);
}

void TestCopyAligned()
{
    alignas(8) u8 src[24];
    alignas(8) u8 dest[24];

    for (usize i = 0; i < 24; ++i) src[i] = static_cast<u8>(i + 10);

    void* result = Memory::CopyAligned(dest, src, 24);
    assert(result == dest);
    for (usize i = 0; i < 24; ++i) assert(dest[i] == src[i]);
}

void TestCopyAlignedUnalignedFallback()
{
    u8 src[24];
    u8 dest[24];

    for (usize i = 0; i < 24; ++i) src[i] = static_cast<u8>(i * 2);

    void* result = Memory::CopyAligned(dest + 1, src + 1, 22);
    assert(result == dest + 1);
    for (usize i = 0; i < 22; ++i) assert(dest[i + 1] == src[i + 1]);
}

int main()
{
    TestSet();
    TestCopy();
    TestMoveOverlapForward();
    TestMoveOverlapBackward();
    TestCompare();
    TestCopyAligned();
    TestCopyAlignedUnalignedFallback();

    printf("All Prism::Memory tests passed.\n");
    return 0;
}
