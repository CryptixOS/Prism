/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

// The format golden corpus. Both Meta/format-golden/Generate.cpp (built
// against the patched fmt 10.2 CryptixOS used to ship) and
// Tests/String/FormatGolden (built against Prism::Format) walk it in the same
// order: every spec of a group against every value of every type of that
// group. The generator records fmt's output in Golden.inc; the test checks
// that Prism::Format produces exactly the same bytes.
//
// Deliberately left out, because Prism is meant to differ there (the regular
// Format test covers what Prism does instead):
// - {:#p} and {:#016p} with integers: the patched fmt prints nothing, Prism
//   prints the integer as an address. CryptixOS formats Pointer::Raw() and
//   physical addresses (integers) this way.
// - {:c} with negative or out-of-range integers.
// - {:L}: the patched fmt has no locale support and prints no separators,
//   Prism groups digits with ','.
// - A width with {:?}: fmt 10.2 pads escaped characters by two columns too
//   many and measures escaped UTF-8 strings in bytes.
// - {:#016p} with pointers: fmt zero-pads in front of the prefix
//   (0000000xdeadbeef), Prism after it (0x000000deadbeef), like {:#016x}.
//
// It is plain C++ with no includes, so both sides can use it. After changing
// it, run Meta/format-golden/generate.sh to refresh Golden.inc.

namespace Golden
{
    // Integers: every spec is used with signed char, short, int, long long,
    // unsigned char, unsigned and unsigned long long (values are converted
    // with static_cast, the same way on both sides).
    inline constexpr const char* kIntegerSpecs[] = {
        "{}",       "{:d}",     "{:x}",      "{:X}",      "{:#x}",
        "{:#X}",    "{:o}",     "{:#o}",     "{:b}",      "{:#b}",
        "{:#B}",    "{:02}",     "{:04}",     "{:02x}",
        "{:#02x}",  "{:#04x}",  "{:#08x}",   "{:#010x}",  "{:#016x}",
        "{:016X}",  "{:+}",     "{: }",      "{:-}",      "{:+08}",
        "{: 06}",   "{:8}",     "{:<8}|",    "{:>8}|",    "{:^8}|",
        "{:*^9}",   "{:*<+7}",  "{:0>6}",    "{:<06}",    "{:#>12x}",
        "{:_^#14b}", "[{:3}]",
        "{0}-{0:x}", "{:#020o}", "{:+#x}",
    };
    inline constexpr long long kIntegerValues[] = {
        0,
        1,
        -1,
        7,
        -7,
        42,
        127,
        -128,
        255,
        1000,
        -1000,
        65535,
        1234567,
        0x7fffffffLL,
        -0x7fffffffLL - 1,
        0xdeadbeefLL,
        0x0123456789abcdefLL,
        0x7fffffffffffffffLL,
        -0x7fffffffffffffffLL - 1,
    };

    // {:c} with integers, only with values that are characters: the patched
    // fmt prints the character of the absolute value for negative ones.
    inline constexpr const char* kCharCodeSpecs[] = {
        "{:c}", "{:>3c}|", "{:*<4c}|", "{:^5c}|",
    };
    inline constexpr int kCharCodes[] = {0, 9, 10, 32, 48, 65, 97, 126, 127};

    // Characters.
    inline constexpr const char* kCharSpecs[] = {
        "{}",   "{:c}",  "{:d}",   "{:x}",   "{:#x}",  "{:3}|", "{:>3}|",
        "{:^5}|", "{:*<4}|", "{:?}", "{:#04x}", "{:b}", "{:o}",
    };
    inline constexpr char kChars[] = {
        'a', 'Z', '0', ' ', '\n', '\t', '\'', '"', '\\', '\x01', '\x7f', '{',
    };

    // Booleans.
    inline constexpr const char* kBoolSpecs[] = {
        "{}",    "{:d}",   "{:s}",   "{:x}",  "{:#x}", "{:>7}|",
        "{:<7}|", "{:^7}|", "{:*^9}", "{:b}", "{:#o}", "{:03}",
    };
    inline constexpr bool kBools[] = {false, true};

    // C strings.
    inline constexpr const char* kStringSpecs[] = {
        "{}",      "{:s}",    "{:8}|",   "{:<8}|",  "{:>8}|",  "{:^8}|",
        "{:.0}|",  "{:.2}|",  "{:.10}|", "{:8.3}|", "{:>8.3}|", "{:*^11}",
        "{:-<6}|", "{:?}",    "[{}]",   "{0}{0}",  "{:^1}|",
    };
    inline constexpr const char* kStrings[] = {
        "",
        "a",
        "str",
        "string",
        "Prism",
        "hello, world",
        "tab\there",
        "line\n",
        "quote\"s",
        "back\\slash",
        "\x01\x1f",
        "{braces}",
        "żółw",
        "日本",
    };

    // Pointers (as addresses; converted to const void* on both sides).
    inline constexpr const char* kPointerSpecs[] = {
        "{}",     "{:p}",     "{:>12}|", "{:<12}|", "{:^12}|",
        "{:#p}",  "{:18p}|", "{:*>20}",
    };
    inline constexpr unsigned long long kPointers[] = {
        0x0, 0x1, 0x1000, 0xdeadbeef, 0xffffffff80000000ULL,
        0xffff800000001234ULL,
    };
} // namespace Golden

// Format strings with several arguments, as an X-macro:
// GOLDEN_MIXED(format string, arguments...). Only arguments both libraries
// format the same way: integers, chars, bools, C strings, nullptr.
#define GOLDEN_MIXED_CASES(GOLDEN_MIXED)                                       \
    GOLDEN_MIXED("plain text")                                                 \
    GOLDEN_MIXED("{{escaped}} }} {{")                                          \
    GOLDEN_MIXED("{} {} {}", 1, "two", '3')                                    \
    GOLDEN_MIXED("{0} {1} {0}", "a", "b")                                      \
    GOLDEN_MIXED("{1:>4}{0:<4}|", 1, 2)                                        \
    GOLDEN_MIXED("{:{}}|", 5, 4)                                               \
    GOLDEN_MIXED("{:>{}}|", "x", 3)                                            \
    GOLDEN_MIXED("{:.{}}|", "abcdef", 2)                                       \
    GOLDEN_MIXED("{0:{1}.{2}}|", "abcdef", 6, 3)                               \
    GOLDEN_MIXED("{:{}.{}}|", "abcdef", 5, 1)                                  \
    GOLDEN_MIXED("{:#0{}x}", 255, 10)                                          \
    GOLDEN_MIXED("{}", nullptr)                                                \
    GOLDEN_MIXED("{:>10}|", nullptr)                                           \
    GOLDEN_MIXED("{}:{}:{}", true, false, 0)                                   \
    GOLDEN_MIXED("[{:>3}] {:<10} {:#x}", 7, "proc", 0x1000)                    \
    GOLDEN_MIXED("pid {} exited with status {:#x}", 42, 0x100)                 \
    GOLDEN_MIXED("{:02}:{:02}:{:02}", 1, 2, 3)                                 \
    GOLDEN_MIXED("{:#04x} {:#04x} {:#04x}", 0x1, 0x22, 0x333)                  \
    GOLDEN_MIXED("{:c}{:c}{:c}", 'P', 'M', 0x21)                               \
    GOLDEN_MIXED("{:→^7}", 1)                                                  \
    GOLDEN_MIXED("{:日<5}|", "ab")                                             \
    GOLDEN_MIXED("{:>{}}|{:<{}}|", 1, 3, 2, 4)
