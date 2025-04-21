/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <string>

#include <Prism/Debug/Log.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringUtils.hpp>

using namespace Prism;
using namespace StringUtils;

void TestGetDigitCount()
{
    for (usize i = 1, value = 1; i < 20; i++)
    {
        auto digitCount = StringUtils::GetDigitCount<usize>(value);
        Log::Info("I: {}, digitCount: {}", i, digitCount);
        assert(digitCount == i);
        value *= 10;
    }
}
void Test_ToString_ToNumber()
{
    // Signed decimal
    assert(ToNumber<i64>("0"_sv) == 0);
    assert(ToNumber<i64>("123"_sv) == 123);
    assert(ToNumber<i64>("-456") == -456);
    assert(ToNumber<i64>("0000123"_sv) == 123);

    assert(ToString<i64>(0) == "0"_sv);
    assert(ToString<i64>(-1) == "-1"_sv);
    assert(ToString<i64>(12345) == "12345"_sv);
    assert(ToString<i64>(-98765) == "-98765"_sv);

    // Unsigned decimal
    assert(ToNumber<u64>("0"_sv) == 0);
    assert(ToNumber<u64>("123456789"_sv) == 123456789);
    assert(ToNumber<u64>("0000000010"_sv) == 10);

    assert(ToString<u64>(0) == "0"_sv);
    assert(ToString<u64>(123456789) == "123456789"_sv);
    assert(ToString<u64>(999999999999ULL) == "999999999999"_sv);

    // Roundtrip signed
    for (i64 i = -10000; i <= 10000; i += 1234)
    {
        String s = ToString<i64>(i);
        assert(ToNumber<i64>(s) == i);
    }

    // Roundtrip unsigned
    for (u64 i = 0; i <= 10000; i += 1234)
    {
        String s = ToString<u64>(i);
        assert(ToNumber<u64>(s) == i);
    }

    // Hexadecimal parsing (ToNumber only)
    assert(ToNumber<u64>("FF"_sv, 16) == 255);
    assert(ToNumber<u64>("0"_sv, 16) == 0);
    assert(ToNumber<u64>("abcdef"_sv, 16) == 0xabcdef);
    assert(ToNumber<u64>("ABCDEF"_sv, 16) == 0xABCDEF);
    assert(ToNumber<u64>("0010"_sv, 16) == 0x10);

    // Large values
    assert(ToNumber<u64>("18446744073709551615"_sv)
           == 18446744073709551615ULL); // Max u64
    assert(ToString<u64>(18446744073709551615ULL) == "18446744073709551615"_sv);

    assert(ToNumber<i64>("-9223372036854775808") == -9223372036854775807LL - 1);
    assert(ToNumber<i64>("9223372036854775807"_sv) == 9223372036854775807LL);
    assert(ToString<i64>(-9223372036854775807LL - 1)
           == "-9223372036854775808"_sv);
    assert(ToString<i64>(9223372036854775807LL) == "9223372036854775807"_sv);
}
int main()
{
    TestGetDigitCount();

    return 0;
}
