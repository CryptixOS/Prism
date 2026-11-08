/*
 * Created by v1tr10l7 on 02.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/RingBuffer.hpp>

using namespace Prism;

TEST_CASE(TestBasicReadWrite){
    RingBuffer buffer(8);

    char       data[] = "ABC";
    EXPECT_EQ((buffer.Write(reinterpret_cast<u8*>(data), 3)), 3);
    printf("Used: %zu\n", buffer.Used());
    printf("Free: %zu\n", buffer.Free());

    EXPECT_EQ(buffer.Used(), 3);
    EXPECT_EQ(buffer.Free(), 5);

    char out[4] = {};
    EXPECT_EQ((buffer.Read(reinterpret_cast<u8*>(out), 3)), 3);
    EXPECT_EQ((memcmp(out, "ABC", 3)), 0);
    EXPECT_EQ(buffer.Used(), 0);
}

TEST_CASE(TestWraparound){
    RingBuffer buffer(4);

    char       data[] = "AB";
    EXPECT_EQ((buffer.Write(reinterpret_cast<u8*>(data), 2)), 2);
    char out[2] = {};
    EXPECT_EQ((buffer.Read(reinterpret_cast<u8*>(out), 2)), 2);

    char more[] = "CD";
    EXPECT_EQ((buffer.Write(reinterpret_cast<u8*>(more), 2)), 2);

    char out2[2] = {};
    EXPECT_EQ((buffer.Read(reinterpret_cast<u8*>(out2), 2)), 2);
    EXPECT_EQ((memcmp(out2, "CD", 2)), 0);
}

TEST_CASE(TestFullBuffer){
    RingBuffer buffer(4);

    char       data[] = "1234";
    EXPECT_EQ((buffer.Write(reinterpret_cast<u8*>(data), 4)), 4);
    EXPECT_EQ(buffer.Free(), 0);
    EXPECT_EQ((buffer.Write(reinterpret_cast<u8*>(data), 1)), 0); // can't write anymore
}

TEST_CASE(TestEmptyBuffer){
    RingBuffer buffer(4);

    char       out[2] = {};
    EXPECT_EQ((buffer.Read(reinterpret_cast<u8*>(out), 2)), 0); // nothing to read
}

/*
void TestZeroCopyWriteRead()
{
    RingBuffer buffer(8);

    void*      writePtr  = nullptr;
    size_t     writeSize = 0;
    buffer.BeginWrite(writePtr, writeSize);
    EXPECT(writeSize > 0);

    char* p = static_cast<char*>(writePtr);
    for (size_t i = 0; i < writeSize; ++i) { p[i] = 'X'; }
    buffer.EndWrite(writeSize);
    EXPECT_EQ(buffer.Used(), writeSize);

    void*  readPtr  = nullptr;
    size_t readSize = 0;
    buffer.BeginRead(readPtr, readSize);
    EXPECT_EQ(readSize, writeSize);
    EXPECT(memset(readPtr, 'X', readSize) == nullptr
           || memcmp(readPtr, "XXXXXXXX", readSize) == 0);

    buffer.EndRead(readSize);
    EXPECT_EQ(buffer.Used(), 0);
}
*/


