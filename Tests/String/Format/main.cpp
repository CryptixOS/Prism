/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/String/Formatter.hpp>
#include <Prism/Test/Test.hpp>

#include <string.h>

using namespace Prism;

// Expected values were produced by fmt for the same format strings and
// arguments (ANSI styles: fmt 10.2 layout).

static void Check(const char* what, StringView got, StringView want)
{
    if (got.Size() == want.Size()
        && memcmp(got.Raw(), want.Raw(), want.Size()) == 0)
        return;
    FAIL(Format("{}\n  got:  [{}]\n  want: [{}]", what, got, want));
}
static void CheckTrue(const char* what, bool condition)
{
    if (!condition) FAIL(what);
}

#define Expect(want, ...) Check(#__VA_ARGS__, Format(__VA_ARGS__), want)

struct Point
{
    int X, Y;
};
template <>
struct Prism::Formatter<Point>
{
    constexpr const char* Parse(FormatParseContext& ctx) { return ctx.begin(); }
    auto                  Format(const Point& p, FormatContext& ctx) const
    {
        return FormatTo(ctx.Out(), "({}, {})", p.X, p.Y);
    }
};

struct Name
{
    const char* N;
};
template <>
struct Prism::Formatter<Name> : Prism::Formatter<StringView>
{
    auto Format(const Name& n, FormatContext& ctx) const
    {
        return Formatter<StringView>::Format(n.N, ctx);
    }
};

namespace NS
{
    enum class E
    {
        eA = 3
    };
    constexpr int FormatAs(E e) { return static_cast<int>(e) * 10; }
} // namespace NS



TEST_CASE(literal_text_and_escapes)
{
    Expect("plain text", "plain text");
    Expect("{escaped} } {", "{{escaped}} }} {{");
}

TEST_CASE(integers)
{
    Expect("0", "{}", 0);
    Expect("-1", "{}", -1);
    Expect("42", "{}", 42u);
    Expect("-9223372036854775808", "{}", -9223372036854775807LL - 1);
    Expect("18446744073709551615", "{}", 18446744073709551615ULL);
    Expect("-5", "{}", (short)-5);
    Expect("200", "{}", (unsigned char)200);
    Expect("-7", "{}", (signed char)-7);
    Expect("ff", "{:x}", 255);
    Expect("FF", "{:X}", 255);
    Expect("0xff", "{:#x}", 255);
    Expect("0XFF", "{:#X}", 255);
    Expect("010", "{:#o}", 8);
    Expect("0", "{:#o}", 0);
    Expect("10", "{:o}", 8);
    Expect("101", "{:b}", 5);
    Expect("0b101", "{:#b}", 5);
    Expect("0B101", "{:#B}", 5);
    Expect("7", "{:d}", 7);
    Expect("-ff", "{:x}", -255);
    Expect("-0xff", "{:#x}", -255);
    Expect("000000ff", "{:08x}", 255);
    Expect("0x000000ff", "{:#010x}", 255);
    Expect("0x000000deadbeef", "{:#016x}", 0xdeadbeefULL);
    Expect("0x01", "{:#04x}", 1);
    Expect("0xab", "{:#02x}", 0xab);
    Expect("05", "{:02}", 5);
    Expect("-005", "{:04}", -5);
    Expect("+5", "{:+}", 5);
    Expect("-5", "{:+}", -5);
    Expect(" 5", "{: }", 5);
    Expect("5", "{:-}", 5);
    Expect("+0000042", "{:+08}", 42);
    Expect("-00003", "{: 06}", -3);
    Expect("    42", "{:6}", 42);
    Expect("42    |", "{:<6}|", 42);
    Expect("  42  |", "{:^6}|", 42);
    Expect("    42|", "{:>6}|", 42);
    Expect("***42****", "{:*^9}", 42);
    Expect("+42***", "{:*<+6}", 42);
    Expect("00007", "{:0>5}", 7);
    Expect("7    ", "{:<05}", 7);
}

TEST_CASE(chars_and_bools)
{
    Expect("A", "{:c}", 65);
    Expect("z", "{:c}", (unsigned char)'z');
    Expect("  B|", "{:>3c}|", 66);
    Expect("x", "{}", 'x');
    Expect("x  |", "{:3}|", 'x');
    Expect("  x|", "{:>3}|", 'x');
    Expect("65", "{:d}", 'A');
    Expect("0x41", "{:#x}", 'A');
    Expect("'x'", "{:?}", 'x');
    Expect("'\\n'", "{:?}", '\n');
    Expect("'\\''", "{:?}", '\'');
    Expect("'\"'", "{:?}", '"');
    Expect("true", "{}", true);
    Expect("false", "{}", false);
    Expect("1", "{:d}", true);
    Expect("  true|", "{:>6}|", true);
    Expect("false", "{:s}", false);
    Expect("0x1", "{:#x}", true);
}

TEST_CASE(strings)
{
    Expect("str", "{}", "str");
    Expect("str     |", "{:8}|", "str");
    Expect("     str|", "{:>8}|", "str");
    Expect("  str   |", "{:^8}|", "str");
    Expect("st", "{:.2}", "string");
    Expect("str     |", "{:8.3}|", "string");
    Expect("|", "{:.0}|", "abc");
    Expect("abc|", "{:.10}|", "abc");
    Expect("\"a\\tb\\n\\\"q\\\" \\\\ '\"", "{:?}", "a\tb\n\"q\" \\ '");
    Expect("\"\\x01\\x7f\"", "{:?}", "\x01\x7f");
    Expect("      \"hi\\n\"|", "{:>12?}|", "hi\n");
    Expect("żół|żó   |", "{:.3}|{:5}|", "żółw", "żó");
    Expect("日本*", "{:*^5}", "日本");
    Expect("→→→1→→→", "{:→^7}", 1);
}

TEST_CASE(pointers)
{
    Expect("0x1234", "{}", (void*)0x1234);
    Expect("0x1234", "{:p}", (void*)0x1234);
    Expect("      0xff|", "{:>10}|", (const void*)0xff);
    Expect("0x0", "{}", nullptr);
}

TEST_CASE(argument_ids_and_dynamic_specs)
{
    Expect("a b a", "{0} {1} {0}", "a", "b");
    Expect("   21   |", "{1:>4}{0:<4}|", 1, 2);
    Expect("   5|", "{:{}}|", 5, 4);
    Expect("  x|", "{:>{}}|", "x", 3);
    Expect("ab|", "{:.{}}|", "abcdef", 2);
    Expect("abc   |", "{0:{1}.{2}}|", "abcdef", 6, 3);
    Expect("a    |", "{:{}.{}}|", "abcdef", 5, 1);
}

TEST_CASE(custom_formatters)
{
    Expect("(1, 2)", "{}", Point{1, 2});
    Expect("      nm|", "{:>8}|", Name{"nm"});
    Expect("  name   |", "{:^9}|", Name{"name"});
    Expect("30 5", "{} {}", NS::E::eA, 5);

}

TEST_CASE(grouping_and_string_types)
{
    Expect("1,234,567", "{:L}", 1234567);
    Expect("-1,000", "{:L}", -1000);
    Expect("12,345,678", "{}", GroupDigits(12345678));
    Expect("view", "{}", StringView("view"));
    Expect("   own", "{:>6}", String("own"));
}

TEST_CASE(named_arguments)
{
    Expect("1    2", "{a} {b:>{w}}", Arg("a", 1), Arg("b", 2), Arg("w", 4));
    {
        using namespace FormatLiterals;
        Expect("1-0xff", "{x}-{y:#x}", "x"_a = 1, "y"_a = 255);
    }
}

TEST_CASE(join)
{
    {
        unsigned char sig[4] = {0x89, 'P', 'N', 'G'};
        Expect("0x89\\0x50\\0x4e\\0x47", "{:#02x}", Join(sig, "\\"));
        int v[3] = {1, 2, 3};
        Expect("[  1,   2,   3]", "[{:>3}]", Join(v + 0, v + 3, ", "));
    }
}

TEST_CASE(helpers)
{
    Expect("3", "{}", Underlying(NS::E::eA));
    Expect("30", "{}", NS::E::eA);
    Check("ToString", ToString(-77), "-77");
    Expect("1 ff", Runtime("{} {:x}"), 1, 255);
    Expect("1 ", Runtime("{} {:x}"), 1);
}

TEST_CASE(styles)
{
    Expect("\x1b[1m\x1b[38;2;255;000;000me1\x1b[0m",
           Fg(TextColor::eRed) | TextEmphasis::eBold, "e{}", 1);
    Expect("\x1b[3m\x1b[4m\x1b[44mx\x1b[0m",
           Bg(TerminalColor::eBlue) | TextEmphasis::eUnderline
               | TextEmphasis::eItalic,
           "x");
    Expect("\x1b[38;2;001;002;003m   7\x1b[0m", "{:>4}",
           Styled(7, Fg(RgbColor(1, 2, 3))));
    Expect("0x1000", "{}", Pointer(0x1000ul));
}

TEST_CASE(output_functions)
{
    {
        char buf[8];
        auto r = FormatToN(buf, sizeof(buf), "{}-{}", 123456, 789);
        Check("FormatToN", StringView(buf, r.Out - buf), "123456-7");
        CheckTrue("FormatToN size", r.Size == 10);
        CheckTrue("FormattedSize", FormattedSize("{:>10}", 1) == 10);
    }
    {
        char  buf[64];
        char* end = FormatTo(buf, "{} {}", "to", 1);
        Check("FormatTo(char*)", StringView(buf, end - buf), "to 1");

        String s = "pre:";
        FormatTo(s, "{:#x}", 16);
        Check("FormatTo(String&)", s, "pre:0x10");
    }
    {
        String big;
        for (int i = 0; i < 300; ++i) big += static_cast<char>('a' + i % 26);
        String formatted = Format("<{}>", big);
        CheckTrue("long output", formatted.Size() == 302
                                     && formatted[0] == '<'
                                     && formatted[301] == '>');
        CheckTrue("long padding", Format("{:>500}", 1).Size() == 500);
    }
    {
        usize total = 0;
        FormatToSink([&](const char*, usize n) { total += n; }, "{:>40}",
                     "sink");
        CheckTrue("FormatToSink", total == 40);
    }

}
