/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <string>

#include <Prism/Debug/Log.hpp>
import Prism.Test.Test;
import Prism.Debug.Log;
import Prism.String.String;
import Prism.String.StringUtils;

using namespace Prism;
using namespace StringUtils;

TEST_CASE(StringUtils_TestGetDigitCount){
    for (usize i = 1, value = 1; i < 20; i++)
    {
        auto digitCount = StringUtils::GetDigitCount<usize>(value);
        Log::Info("I: {}, digitCount: {}", i, digitCount);
        EXPECT_EQ(digitCount, i);
        value *= 10;
    }
}
void StringUtils_Test_ToString_ToNumber()
{
    // Signed decimal
    EXPECT(ToNumber<i64>("0"_sv) == 0);
    EXPECT(ToNumber<i64>("123"_sv) == 123);
    EXPECT(ToNumber<i64>("-456") == -456);
    EXPECT(ToNumber<i64>("0000123"_sv) == 123);

    EXPECT(ToString<i64>(0) == "0"_sv);
    EXPECT(ToString<i64>(-1) == "-1"_sv);
    EXPECT(ToString<i64>(12345) == "12345"_sv);
    EXPECT(ToString<i64>(-98765) == "-98765"_sv);

    // Unsigned decimal
    EXPECT(ToNumber<u64>("0"_sv) == 0);
    EXPECT(ToNumber<u64>("123456789"_sv) == 123456789);
    EXPECT(ToNumber<u64>("0000000010"_sv) == 10);

    EXPECT(ToString<u64>(0) == "0"_sv);
    EXPECT(ToString<u64>(123456789) == "123456789"_sv);
    EXPECT(ToString<u64>(999999999999ULL) == "999999999999"_sv);

    // Roundtrip signed
    for (i64 i = -10000; i <= 10000; i += 1234)
    {
        String s = ToString<i64>(i);
        EXPECT(ToNumber<i64>(s) == i);
    }

    // Roundtrip unsigned
    for (u64 i = 0; i <= 10000; i += 1234)
    {
        String s = ToString<u64>(i);
        EXPECT(ToNumber<u64>(s) == i);
    }

    // Hexadecimal parsing (ToNumber only)
    EXPECT(ToNumber<u64>("FF"_sv, 16) == 255);
    EXPECT(ToNumber<u64>("0"_sv, 16) == 0);
    EXPECT(ToNumber<u64>("abcdef"_sv, 16) == 0xabcdef);
    EXPECT(ToNumber<u64>("ABCDEF"_sv, 16) == 0xABCDEF);
    EXPECT(ToNumber<u64>("0010"_sv, 16) == 0x10);

    // Large values
    EXPECT(ToNumber<u64>("18446744073709551615"_sv)
           == 18446744073709551615ULL); // Max u64
    EXPECT(ToString<u64>(18446744073709551615ULL) == "18446744073709551615"_sv);

    EXPECT(ToNumber<i64>("-9223372036854775808") == -9223372036854775807LL - 1);
    EXPECT(ToNumber<i64>("9223372036854775807"_sv) == 9223372036854775807LL);
    EXPECT(ToString<i64>(-9223372036854775807LL - 1)
           == "-9223372036854775808"_sv);
    EXPECT(ToString<i64>(9223372036854775807LL) == "9223372036854775807"_sv);

    EXPECT(ToNumber<u64>("177"_sv, 8) == 127);
    EXPECT(ToNumber<u64>("999"_sv, 8) == 511);
    EXPECT(ToNumber<u64>("242"_sv, 8) == 162);
}

