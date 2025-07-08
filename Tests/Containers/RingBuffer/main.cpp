/*
 * Created by v1tr10l7 on 02.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/RingBuffer.hpp>
#include <cassert>

using namespace Prism;

void TestBasicReadWrite()
{
    RingBuffer buffer(8);

    char       data[] = "ABC";
    assert(buffer.Write(reinterpret_cast<u8*>(data), 3) == 3);
    printf("Used: %zu\n", buffer.Used());
    printf("Free: %zu\n", buffer.Free());

    assert(buffer.Used() == 3);
    assert(buffer.Free() == 5);

    char out[4] = {};
    assert(buffer.Read(reinterpret_cast<u8*>(out), 3) == 3);
    assert(memcmp(out, "ABC", 3) == 0);
    assert(buffer.Used() == 0);
}

void TestWraparound()
{
    RingBuffer buffer(4);

    char       data[] = "AB";
    assert(buffer.Write(reinterpret_cast<u8*>(data), 2) == 2);
    char out[2] = {};
    assert(buffer.Read(reinterpret_cast<u8*>(out), 2) == 2);

    char more[] = "CD";
    assert(buffer.Write(reinterpret_cast<u8*>(more), 2) == 2);

    char out2[2] = {};
    assert(buffer.Read(reinterpret_cast<u8*>(out2), 2) == 2);
    assert(memcmp(out2, "CD", 2) == 0);
}

void TestFullBuffer()
{
    RingBuffer buffer(4);

    char       data[] = "1234";
    assert(buffer.Write(reinterpret_cast<u8*>(data), 4) == 4);
    assert(buffer.Free() == 0);
    assert(buffer.Write(reinterpret_cast<u8*>(data), 1)
           == 0); // can't write anymore
}

void TestEmptyBuffer()
{
    RingBuffer buffer(4);

    char       out[2] = {};
    assert(buffer.Read(reinterpret_cast<u8*>(out), 2) == 0); // nothing to read
}

/*
void TestZeroCopyWriteRead()
{
    RingBuffer buffer(8);

    void*      writePtr  = nullptr;
    size_t     writeSize = 0;
    buffer.BeginWrite(writePtr, writeSize);
    assert(writeSize > 0);

    char* p = static_cast<char*>(writePtr);
    for (size_t i = 0; i < writeSize; ++i) { p[i] = 'X'; }
    buffer.EndWrite(writeSize);
    assert(buffer.Used() == writeSize);

    void*  readPtr  = nullptr;
    size_t readSize = 0;
    buffer.BeginRead(readPtr, readSize);
    assert(readSize == writeSize);
    assert(memset(readPtr, 'X', readSize) == nullptr
           || memcmp(readPtr, "XXXXXXXX", readSize) == 0);

    buffer.EndRead(readSize);
    assert(buffer.Used() == 0);
}
*/

int main()
{
    TestBasicReadWrite();
    TestWraparound();
    TestFullBuffer();
    TestEmptyBuffer();
    // TestZeroCopyWriteRead();

    return 0;
}
