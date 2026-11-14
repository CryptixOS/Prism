/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// Prism strings and formatting vs std::string, std::string_view,
// std::format and <charconv>. See Common/Bench.hpp for the flags.

#include "../Common/Bench.hpp"

#include <charconv>
#include <format>
#include <string>
#include <string_view>
#include <vector>

import Prism.Containers.UnorderedMap;
import Prism.String.Formatter;
import Prism.String.String;
import Prism.String.StringBuilder;
import Prism.String.StringUtils;
import Prism.String.StringView;

using Bench::Name;
using Bench::Opaque;
using Bench::Runner;
using Bench::u64;

// Both sides hash their output the same way, so checksums match only when
// the bytes do.
static u64 Digest(const char* data, size_t size)
{
    u64 h = 0xcbf29ce484222325ull ^ size;
    for (size_t i = 0; i < size; ++i) h = (h ^ static_cast<unsigned char>(data[i])) * 0x100000001b3ull;
    return h;
}
static u64 Digest(const Prism::String& s) { return Digest(s.Raw(), s.Size()); }
static u64 Digest(Prism::StringView s) { return Digest(s.Raw(), s.Size()); }
static u64 Digest(std::string_view s) { return Digest(s.data(), s.size()); }

static std::string RandomText(size_t size, u64 seed, int alphabet = 26)
{
    Bench::Rng  rng(seed);
    std::string text(size, ' ');
    for (auto& c : text) c = char('a' + rng.Below(alphabet));
    return text;
}

static const char kShort[] = "hello prism";                 // fits SSO
static const char kLong[]  = "the quick brown fox jumps over the lazy dog, "
                             "twice over, with feeling";   // heap

// ============================================================================
// String

static void RegisterString()
{
    for (const char* text : {kShort, kLong})
    {
        const char* label = text == kShort ? "short" : "long";
        Bench::Add("String", Name("Construct", label), [text](Runner& r) {
            r.Compare(
                1, [text] { return Digest(Prism::String(Opaque(text))); },
                [text] { return Digest(std::string(Opaque(text))); });
        });
        Bench::Add("String", Name("Copy", label), [text](Runner& r) {
            Prism::String ps(text);
            std::string   ss(text);
            r.Compare(
                1,
                [&] {
                    Prism::String copy(Opaque(&ps)[0]);
                    return u64(copy.Size() + copy[0]);
                },
                [&] {
                    std::string copy(Opaque(&ss)[0]);
                    return u64(copy.size() + copy[0]);
                });
        });
        Bench::Add("String", Name("Equal", label), [text](Runner& r) {
            Prism::String pa(text), pb(text);
            std::string   sa(text), sb(text);
            r.Compare(
                1, [&] { return u64(Opaque(&pa)[0] == Opaque(&pb)[0]); },
                [&] { return u64(Opaque(&sa)[0] == Opaque(&sb)[0]); });
        });
    }
    for (size_t n : {16, 256, 4096})
    {
        Bench::Add("String", Name("AppendChar", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::String s;
                    for (size_t i = 0; i < Opaque(n); ++i) s += char('a' + i % 26);
                    return Digest(s);
                },
                [n] {
                    std::string s;
                    for (size_t i = 0; i < Opaque(n); ++i) s += char('a' + i % 26);
                    return Digest(s);
                });
        });
        Bench::Add("String", Name("AppendView/12B", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::String     s;
                    Prism::StringView piece("prism-string");
                    for (size_t i = 0; i < Opaque(n); ++i) s += piece;
                    return Digest(s);
                },
                [n] {
                    std::string      s;
                    std::string_view piece("prism-string");
                    for (size_t i = 0; i < Opaque(n); ++i) s += piece;
                    return Digest(s);
                });
        });
    }
    for (size_t n : {64, 4096, 65536})
    {
        // The character is only at the end: a full scan.
        Bench::Add("String", Name("FindChar", n), [n](Runner& r) {
            std::string   text = RandomText(n, 3, 25) + 'z';
            Prism::String ps(text.c_str());
            r.Bytes(double(text.size()));
            r.Compare(
                1, [&] { return u64(ps.Find('z')); },
                [&] { return u64(text.find('z')); });
        });
        Bench::Add("String", Name("RFindChar", n), [n](Runner& r) {
            std::string   text = 'z' + RandomText(n, 3, 25);
            Prism::String ps(text.c_str());
            r.Bytes(double(text.size()));
            r.Compare(
                1, [&] { return u64(ps.RFind('z')); },
                [&] { return u64(text.rfind('z')); });
        });
        // A 12-byte needle at the end of random text.
        Bench::Add("String", Name("FindSubstring/random", n), [n](Runner& r) {
            std::string   needle = "prismneedle!";
            std::string   text   = RandomText(n, 4) + needle;
            Prism::String ps(text.c_str());
            Prism::String pn(needle.c_str());
            r.Bytes(double(text.size()));
            r.Compare(
                1, [&] { return u64(ps.Find(pn)); },
                [&] { return u64(text.find(needle)); });
        });
        // aaaa...ab in aaaa...: worst case for a naive search.
        Bench::Add("String", Name("FindSubstring/periodic", n), [n](Runner& r) {
            std::string   needle = std::string(31, 'a') + 'b';
            std::string   text   = std::string(n, 'a') + needle;
            Prism::String ps(text.c_str());
            Prism::String pn(needle.c_str());
            r.Bytes(double(text.size()));
            r.Compare(
                1, [&] { return u64(ps.Find(pn)); },
                [&] { return u64(text.find(needle)); });
        });
    }
    Bench::Add("String", "Substr/long", [](Runner& r) {
        Prism::String ps(kLong);
        std::string   ss(kLong);
        r.Compare(
            1, [&] { return Digest(ps.Substr(Opaque(4), 40)); },
            [&] { return Digest(ss.substr(Opaque(4), 40)); });
    });
    Bench::Add("String", "Split/csv-64-fields", [](Runner& r) {
        std::string line;
        for (int i = 0; i < 64; ++i) line += "field" + std::to_string(i * 37) + ',';
        Prism::String ps(line.c_str());
        r.Compare(
            64,
            [&] {
                auto parts = ps.Split(',');
                u64  h     = parts.Size();
                for (auto& p : parts) h = h * 31 + p.Size();
                return h;
            },
            [&] {
                std::vector<std::string> parts;
                size_t                   start = 0;
                for (size_t comma; (comma = line.find(',', start)) != std::string::npos;
                     start = comma + 1)
                    if (comma > start) parts.emplace_back(line, start, comma - start);
                if (start < line.size()) parts.emplace_back(line, start);
                u64 h = parts.size();
                for (auto& p : parts) h = h * 31 + p.size();
                return h;
            });
    });
}

// ============================================================================
// StringView

static void RegisterStringView()
{
    Bench::Add("StringView", "Compare/long", [](Runner& r) {
        std::string      a = kLong, b = kLong;
        b.back()           = '!';
        Prism::StringView pa(a.c_str()), pb(b.c_str());
        std::string_view  sa(a), sb(b);
        r.Compare(
            1, [&] { return u64(Opaque(pa).Compare(Opaque(pb)) < 0); },
            [&] { return u64(Opaque(sa).compare(Opaque(sb)) < 0); });
    });
    Bench::Add("StringView", "StartsWith", [](Runner& r) {
        Prism::StringView pv(kLong);
        std::string_view  sv(kLong);
        r.Compare(
            1, [&] { return u64(Opaque(pv).StartsWith("the quick brown")); },
            [&] { return u64(Opaque(sv).starts_with("the quick brown")); });
    });
    Bench::Add("StringView", "FromCString/long", [](Runner& r) {
        r.Compare(
            1, [] { return u64(Prism::StringView(Opaque(kLong)).Size()); },
            [] { return u64(std::string_view(Opaque(kLong)).size()); });
    });
    Bench::Add("StringView", "Trim", [](Runner& r) {
        const char* padded = "   \t  some padded value  \n ";
        r.Compare(
            1, [&] { return Digest(Prism::StringView(Opaque(padded)).Trim()); },
            [&] {
                std::string_view v(Opaque(padded));
                const char*      ws    = " \t\n\r\f\v";
                size_t           first = v.find_first_not_of(ws);
                if (first == std::string_view::npos) return Digest(std::string_view{});
                size_t last = v.find_last_not_of(ws);
                return Digest(v.substr(first, last - first + 1));
            });
    });
    for (size_t n : {16, 64, 1024})
    {
        // The hashers UnorderedMap uses by default. Different hash
        // functions, so only the speed is comparable.
        Bench::Add("StringView", Name("Hash", n), [n](Runner& r) {
            std::string       text = RandomText(n, 5);
            Prism::StringView pv(text.c_str(), text.size());
            std::string_view  sv(text);
            r.Bytes(double(n));
            r.Compare(
                1, [&] { return u64(Prism::Hasher<Prism::StringView>{}(Opaque(pv))); },
                [&] { return u64(std::hash<std::string_view>{}(Opaque(sv))); },
                Bench::NoCheck);
        });
        Bench::Add("String", Name("Hash", n), [n](Runner& r) {
            std::string   text = RandomText(n, 5);
            Prism::String ps(text.c_str());
            r.Bytes(double(n));
            r.Compare(
                1, [&] { return u64(Prism::Hasher<Prism::String>{}(Opaque(&ps)[0])); },
                [&] { return u64(std::hash<std::string>{}(Opaque(&text)[0])); },
                Bench::NoCheck);
        });
    }
}

// ============================================================================
// StringUtils: ToNumber / ToString vs <charconv>

static void RegisterStringUtils()
{
    std::vector<std::string> numbers;
    {
        Bench::Rng rng(6);
        for (int i = 0; i < 256; ++i)
            numbers.push_back(std::to_string(rng.Next() >> (rng.Below(60))));
    }
    Bench::Add("StringUtils", "ToNumber/u64", [numbers](Runner& r) {
        std::vector<Prism::StringView> views;
        for (auto& s : numbers) views.emplace_back(s.c_str(), s.size());
        r.Compare(
            numbers.size(),
            [&] {
                u64 sum = 0;
                for (auto v : views) sum += Prism::StringUtils::ToNumber<u64>(v);
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (auto& s : numbers)
                {
                    u64 value = 0;
                    std::from_chars(s.data(), s.data() + s.size(), value);
                    sum += value;
                }
                return sum;
            });
    });
    Bench::Add("StringUtils", "ToNumber/i64-negative", [numbers](Runner& r) {
        std::vector<std::string> negative;
        for (auto& s : numbers)
            if (s.size() < 19) negative.push_back('-' + s);
        std::vector<Prism::StringView> views;
        for (auto& s : negative) views.emplace_back(s.c_str(), s.size());
        r.Compare(
            negative.size(),
            [&] {
                u64 sum = 0;
                for (auto v : views) sum += u64(Prism::StringUtils::ToNumber<Bench::i64>(v));
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (auto& s : negative)
                {
                    std::int64_t value = 0;
                    std::from_chars(s.data(), s.data() + s.size(), value);
                    sum += u64(value);
                }
                return sum;
            });
    });
    Bench::Add("StringUtils", "ToNumber/hex", [](Runner& r) {
        std::vector<std::string> hex;
        Bench::Rng               rng(7);
        for (int i = 0; i < 256; ++i)
        {
            char buffer[24];
            auto end = std::to_chars(buffer, buffer + 24, rng.Next() >> rng.Below(60), 16).ptr;
            hex.emplace_back(buffer, end);
        }
        std::vector<Prism::StringView> views;
        for (auto& s : hex) views.emplace_back(s.c_str(), s.size());
        r.Compare(
            hex.size(),
            [&] {
                u64 sum = 0;
                for (auto v : views) sum += Prism::StringUtils::ToNumber<u64>(v, 16);
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (auto& s : hex)
                {
                    u64 value = 0;
                    std::from_chars(s.data(), s.data() + s.size(), value, 16);
                    sum += value;
                }
                return sum;
            });
    });
    Bench::Add("StringUtils", "ToString/u64-buffer", [](Runner& r) {
        std::vector<u64> values;
        Bench::Rng       rng(8);
        for (int i = 0; i < 256; ++i) values.push_back(rng.Next() >> rng.Below(60));
        r.Compare(
            values.size(),
            [&] {
                char buffer[32];
                u64  h = 0;
                for (u64 v : values)
                    h ^= Digest(Prism::StringUtils::ToString(v, buffer));
                return h;
            },
            [&] {
                char buffer[32];
                u64  h = 0;
                for (u64 v : values)
                {
                    auto end = std::to_chars(buffer, buffer + 32, v).ptr;
                    h ^= Digest(buffer, size_t(end - buffer));
                }
                return h;
            });
    });
    Bench::Add("StringUtils", "ToString/u64-string", [](Runner& r) {
        std::vector<u64> values;
        Bench::Rng       rng(9);
        for (int i = 0; i < 256; ++i) values.push_back(rng.Next() >> rng.Below(60));
        r.Compare(
            values.size(),
            [&] {
                u64 h = 0;
                for (u64 v : values) h ^= Digest(Prism::StringUtils::ToString(v));
                return h;
            },
            [&] {
                u64 h = 0;
                for (u64 v : values) h ^= Digest(std::to_string(v));
                return h;
            });
    });
}

// ============================================================================
// StringBuilder vs std::string appends

static void RegisterStringBuilder()
{
    for (size_t n : {16, 1024})
    {
        Bench::Add("StringBuilder", Name("AppendToString", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::StringBuilder<char> sb;
                    for (size_t i = 0; i < Opaque(n); ++i)
                    {
                        sb.Append("key=");
                        sb.Append(u64(i));
                        sb.Append(';');
                    }
                    return Digest(sb.ToString());
                },
                [n] {
                    std::string s;
                    for (size_t i = 0; i < Opaque(n); ++i)
                    {
                        s += "key=";
                        s += std::to_string(i);
                        s += ';';
                    }
                    return Digest(s);
                });
        });
    }
}

// ============================================================================
// Format vs std::format

template <typename... Args>
static void FormatCase(const char* name, Prism::FormatString<Args...> pf,
                       std::format_string<Args...> sf, Args... args)
{
    Bench::Add("Format", name, [=](Runner& r) {
        r.Compare(
            1, [=] { return Digest(Prism::Format(pf, Opaque(args)...)); },
            [=] { return Digest(std::format(sf, Opaque(args)...)); });
    });
}

static void RegisterFormat()
{
    FormatCase("int", "{}", "{}", 123456789);
    FormatCase("u64-max", "{}", "{}", ~u64(0));
    FormatCase("hex-prefixed", "{:#x}", "{:#x}", 0xdeadbeefu);
    FormatCase("padded", "[{:>12}] [{:<8}] [{:^9}]", "[{:>12}] [{:<8}] [{:^9}]",
               42, 7, -3);
    FormatCase("zero-pad", "{:016x}", "{:016x}", u64(0xfeedface));
    FormatCase("string", "name={} value={}", "name={} value={}", "prism", "bench");
    FormatCase("mixed", "{}: {} + {:#x} = {:>8}", "{}: {} + {:#x} = {:>8}", "sum",
               1234, 255, 'c');
    // No floating-point cases: Prism::Format does not format floats.
    FormatCase("pointer", "{}", "{}", static_cast<const void*>(kLong));
    FormatCase("bool", "{} {}", "{} {}", true, false);
    FormatCase("long-literal",
               "a fairly long format string with only one {} in the middle of it",
               "a fairly long format string with only one {} in the middle of it",
               99);

    Bench::Add("Format", "FormatToN/char-buffer", [](Runner& r) {
        r.Compare(
            1,
            [] {
                char buffer[64];
                auto result = Prism::FormatToN(buffer, sizeof(buffer), "{}:{:#x}:{}",
                                               Opaque(42), Opaque(255), "x");
                return Digest(buffer, result.Size);
            },
            [] {
                char buffer[64];
                auto result = std::format_to_n(buffer, sizeof(buffer), "{}:{:#x}:{}",
                                               Opaque(42), Opaque(255), "x");
                return Digest(buffer, size_t(result.size));
            });
    });
}

int main(int argc, char** argv)
{
    RegisterString();
    RegisterStringView();
    RegisterStringUtils();
    RegisterStringBuilder();
    RegisterFormat();
    return Bench::Main(argc, argv, "String");
}
