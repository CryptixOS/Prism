/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/String/Formatter.hpp>
#include <Prism/String/String.hpp>
#include <Prism/Utility/Path.hpp>
#include <cassert>
#include <ctime>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace Prism;
using namespace Prism::Literals;

TEST(BasicString, String_TestConstruction)
{
    String empty;
    ASSERT_TRUE(empty.Empty());
    ASSERT_EQ(empty.Size(), 0);

    String repeated(5, 'x');
    ASSERT_EQ(repeated, "xxxxx"_s);

    const char* raw = "hello";
    String      fromRaw(raw);
    ASSERT_EQ(fromRaw, "hello"_s);

    String fromView = "world"_s;
    ASSERT_EQ(fromView, "world"_s);
}

TEST(BasicString, String_TestIteration)
{
    String      s = "abc"_s;

    std::string built;
    for (char c : s) built += c;

    ASSERT_EQ(built, "abc");
}

TEST(BasicString, String_TestAccessors)
{
    String s = "hello"_s;

    ASSERT_EQ(s.At(0), 'h');
    ASSERT_EQ(s[1], 'e');
    ASSERT_EQ(s.Front(), 'h');
    ASSERT_EQ(s.Back(), 'o');
}

TEST(BasicString, String_TestFind)
{
    String s = "hello world"_s;

    ASSERT_EQ(s.Find("world"), 6);
    ASSERT_EQ(s.Find('o'), 4);
    ASSERT_EQ(s.RFind('o'), 7);
    s = "hello world"_s;

    // Find
    ASSERT_EQ(s.Find("hello"), 0);
    ASSERT_EQ(s.Find("world"), 6);
    ASSERT_EQ(s.Find("o"), 4);
    ASSERT_EQ(s.Find("notfound"), String::NPos);
    ASSERT_EQ(s.Find('l'), 2);

    // RFind
    ASSERT_EQ(s.RFind("hello"), 0);
    ASSERT_EQ(s.RFind("world"), 6);
    ASSERT_EQ(s.RFind("o"), 7);
    ASSERT_EQ(s.RFind('l'), 9);
    ASSERT_EQ(s.RFind("notfound"), String::NPos);

    // FindFirstOf
    ASSERT_EQ(s.FindFirstOf("aeiou"_s), 1); // 'e'
    ASSERT_EQ(s.FindFirstOf("xyz"_s), String::NPos);
    ASSERT_EQ(s.FindFirstOf('o'), 4);

    // FindFirstNotOf
    ASSERT_EQ(s.FindFirstNotOf("hel"_s), 4); // 'o'
    ASSERT_TRUE(s.FindFirstNotOf("hello wrd"_s)
                == String::NPos); // all characters matched
    ASSERT_EQ(s.FindFirstNotOf('h'), 1);

    // FindLastOf
    ASSERT_EQ(s.FindLastOf("ol"), 9); // last 'l'
    ASSERT_EQ(s.FindLastOf("xyz"), String::NPos);
    ASSERT_EQ(s.FindLastOf('l'), 9);

    // FindLastNotOf
    // ASSERT_EQ(s.FindLastNotOf("d"_s) ,  9); // 'l'
    // ASSERT_TRUE(s.FindLastNotOf("helo wrd"_s)
    //       == String::NPos); // all characters matched
    // ASSERT_EQ(s.FindLastNotOf('d') ,  9);
}

TEST(BasicString, String_TestComparisons)
{
    String a = "abc"_s;
    String b = "abc"_s;
    String c = "abd"_s;

    ASSERT_EQ(a, b);
    ASSERT_TRUE(a != c);
    ASSERT_EQ((a <=> c), std::strong_ordering::less);
}

TEST(BasicString, String_TestSubstringAndResize)
{
    String s   = "abcdef"_s;

    String sub = s.Substr(1, 3);
    ASSERT_EQ(sub, "bcd"_s);

    s.Resize(3);
    printf("s1: %s\n", s.Raw());
    printf("s2: %s\n", "abc"_s.Raw());
    ASSERT_EQ(s, "abc"_s);

    s.Resize(5, 'x');
    ASSERT_EQ(s, "abcxx"_s);
}

TEST(BasicString, String_TestCopyAndSwap)
{
    String a = "abc"_s;
    String b = "xyz"_s;

    a.Swap(b);
    ASSERT_EQ(a, "xyz"_s);
    ASSERT_EQ(b, "abc"_s);

    a = b;
    ASSERT_EQ(a, "abc"_s);
}

Path String_TestPath(PathView path)
{
    auto segments = path.Split();
    auto strings  = StringView(path).Split('/');
    for (usize i = 0; i < segments.Size(); i++)
    {
        String              segment = segments[i];
        StringView          string  = strings[i];
        // StringBuilder<char> sb;
        // FormatTo(sb, "VFS: '{:.{}}' == '{:.{}}' {}\n", segment);
        // FormatTo(sb, "VFS: '{:.{}}' == '{:.{}}' {}\n", segment.Raw(),
        //          segment.Size(), string.Raw(), string.Size(),
        //          (segment == string));

        std::cout << "VFS: '" << std::setprecision(segment.Size())
                  << segment.Raw() << "' == '"
                  << std::setprecision(string.Size()) << string.Raw() << "'"
                  << (segment == string) << "\n";

        if (i == segments.Size() - 1) return Path(segment);
    }

    return !segments.Empty() ? segments.Front() : ":(";
}
TEST(BasicString, String_TestSplit)
{
    PathView path         = "/usr/sbin/string/someother/anyway/yo/hello.txt"_sv;
    auto     newPath      = String_TestPath(path);
    auto     result       = String_TestPath(path);
    auto     concatenated = String(result.View());
    concatenated += String(newPath.View());
    String_TestPath(concatenated);
}
TEST(BasicString, String_TestMove)
{
    String str   = "Hello";
    auto   other = Move(str);
    ASSERT_TRUE(str.Empty());
    ASSERT_EQ(other.Size(), 5);

    String next(Move(other));
    ASSERT_TRUE(other.Empty());
    ASSERT_EQ(next.Size(), 5);
}

TEST(BasicString, String_TestBasicStringAssignments)
{
    using String = BasicString<char>;

    // Test copy assignment
    {
        String a = "Hello";
        String b;
        b = a;
        ASSERT_EQ(b, "Hello");
        ASSERT_EQ(a, "Hello");
        ASSERT_EQ(b.Size(), 5);
    }

    // Test move assignment
    {
        String a = "World";
        String b;
        b = Move(a);
        ASSERT_EQ(b, "World");
        // We can't assume `a` still contains valid data after move, only that
        // it's safe to destroy.
        ASSERT_EQ(b.Size(), 5);
    }

    // Test assignment from const char*
    {
        String a;
        a = "Test123";
        ASSERT_EQ(a, "Test123");
        ASSERT_EQ(a.Size(), 7);
    }

    // Test assignment from a single character
    {
        String a;
        a = 'Z';
        ASSERT_EQ(a, "Z");
        ASSERT_EQ(a.Size(), 1);
    }

    // Test assignment from StringView-like object
    {
        String     a = "initial";
        StringView view("stringview");

        a = view;
        ASSERT_EQ(a, "stringview");
        ASSERT_EQ(a.Size(), 10);
    }
}

TEST(BasicString, String_TestBasicStringAddition)
{
    using String = BasicString<char>;

    // Assuming you have operator+(const BasicString&, const BasicString&)
    {
        String a = "Hello";
        String b = "World";
        String c = a + b;

        ASSERT_EQ(a, "Hello"); // Must stay unchanged
        ASSERT_EQ(b, "World"); // Must stay unchanged
        ASSERT_EQ(c, "HelloWorld");
        ASSERT_EQ(c.Size(), 10);
    }

    // Addition with const char* (if supported)
    {
        String a = "Foo";
        String b = a + "Bar";

        ASSERT_EQ(a, "Foo"); // a must stay unchanged
        ASSERT_EQ(b, "FooBar");
        ASSERT_EQ(b.Size(), 6);
    }

    // Addition with empty string
    {
        String a = "NonEmpty";
        String b;
        String c = a + b;

        ASSERT_EQ(c, "NonEmpty");
        ASSERT_EQ(c.Size(), 8);
    }
}
// Random helper
char String_RandomChar()
{
    const char charset[]
        = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    return charset[rand() % (sizeof(charset) - 1)];
}

String String_RandomString(usize length = 100)
{
    String str;
    str.Reserve(length);
    for (usize i = 0; i < length; i++) str += String_RandomChar();
    return str;
}

// Fuzz test
TEST(BasicString, String_FuzzTestBasicString)
{
    srand(static_cast<unsigned>(time(nullptr))); // Seed random

    for (int iteration = 0; iteration < 1000; iteration++)
    {
        usize  len1 = rand() % 100;
        usize  len2 = rand() % 100;

        String a    = String_RandomString(len1);
        String b    = String_RandomString(len2);

        // Copy assignment
        String copy = a;
        ASSERT_EQ(copy, a);
        ASSERT_EQ(copy.Size(), a.Size());

        // Move assignment
        String move = Move(b);
        ASSERT_EQ(move.Size(), len2); // b was random of size len2

        // Assignment from const char*
        const char* literal = "LiteralString";
        a                   = literal;
        ASSERT_EQ(a, "LiteralString");
        ASSERT_EQ(a.Size(), strlen(literal));

        // Assignment from single char
        b = 'X';
        ASSERT_EQ(b, "X");
        ASSERT_EQ(b.Size(), 1);

        // Assignment from StringView
        StringView view("ViewContent");
        a = view;
        ASSERT_EQ(a, "ViewContent");
        ASSERT_EQ(a.Size(), strlen("ViewContent"));

        // Operator+
        String combined = a + b;
        ASSERT_EQ(combined, "ViewContentX");
        ASSERT_EQ(combined.Size(), strlen("ViewContentX"));
    }
}

TEST(BasicString, String_FuzzTestBasicString2)
{
    srand(static_cast<unsigned>(time(nullptr))); // Seed random

    for (int iteration = 0; iteration < 5000; iteration++) // More iterations!
    {
        usize  len1 = rand() % 100;
        usize  len2 = rand() % 100;

        String a    = String_RandomString(len1);
        String b    = String_RandomString(len2);

        // --- Copy assignment ---
        String copy = a;
        ASSERT_EQ(copy, a);
        ASSERT_EQ(copy.Size(), a.Size());

        // --- Self-copy assignment ---
        a = a;
        ASSERT_EQ(a, copy);

        // --- Move assignment ---
        String move = Move(b);
        ASSERT_EQ(move.Size(), len2); // b was random of size len2

        // --- Assignment from const char* ---
        const char* literal = "LiteralString";
        a                   = literal;
        ASSERT_EQ(a, "LiteralString");
        ASSERT_EQ(a.Size(), strlen(literal));

        // --- Assignment from single char ---
        b = 'X';
        ASSERT_EQ(b, "X");
        ASSERT_EQ(b.Size(), 1);

        // --- Assignment from StringView ---
        StringView view("ViewContent");
        a = view;
        ASSERT_EQ(a, "ViewContent");
        ASSERT_EQ(a.Size(), strlen("ViewContent"));

        // --- Operator+ ---
        String combined = a + b;
        ASSERT_EQ(combined, "ViewContentX");
        ASSERT_EQ(combined.Size(), strlen("ViewContentX"));

        // --- Operator+= ---
        a += b;
        ASSERT_EQ(a, "ViewContentX");
        ASSERT_EQ(a.Size(), strlen("ViewContentX"));

        // --- Assignment with empty string ---
        a = "";
        ASSERT_TRUE(a.Empty());
        ASSERT_EQ(a.Size(), 0);

        // --- Concatenation with empty ---
        String c         = String_RandomString(rand() % 20);
        String d         = "";
        String combined2 = c + d;
        ASSERT_EQ(combined2, c);

        combined2 = d + c;
        ASSERT_EQ(combined2, c);

        // --- Fuzz: Random characters append ---
        usize  randomPushCount = rand() % 50;
        String fuzzString;
        fuzzString.Reserve(randomPushCount);
        for (usize j = 0; j < randomPushCount; j++)
        {
            char randomChar = String_RandomChar();
            fuzzString += randomChar;
            ASSERT_EQ(fuzzString.Size(), j + 1);
            ASSERT_EQ(fuzzString.Back(), randomChar);
        }
    }
}

TEST(BasicString, String_MegaFuzzTestBasicString)
{
    srand(static_cast<unsigned>(time(nullptr)));

    std::vector<String> pool; // Pool of random strings

    for (int iteration = 0; iteration < 10000; iteration++) // Big stress
    {
        int op = rand() % 10;

        if (pool.empty() || op < 2) // Force-create strings early
        {
            pool.push_back(String_RandomString());
            continue;
        }

        int    idxA = rand() % pool.size();
        int    idxB = rand() % pool.size();

        String a    = pool[idxA];
        String b    = pool[idxB];
        if (a.Raw() == b.Raw())
        {
            pool.push_back(String_RandomString());
            b       = pool.back();
            pool[0] = pool[1];
            continue;
        }

        switch (op)
        {
            case 0: // Copy assign
            {
                String copy = a;
                ASSERT_EQ(copy, a);
                a = copy;
                ASSERT_EQ(a, copy);
                break;
            }
            case 1: // Move assign
            {
                String tmp   = b;
                String moved = Move(tmp);
                ASSERT_EQ(moved, b);
                break;
            }
            case 2: // Assignment from const char*
            {
                const char* literals[]
                    = {"Test", "Mega", "Fuzz", "", "String!"};
                const char* lit = literals[rand() % 5];
                a               = lit;
                ASSERT_EQ(a, lit);
                break;
            }
            case 3: // Assignment from char
            {
                char ch = String_RandomChar();
                a       = ch;
                ASSERT_EQ(a.Size(), 1);
                ASSERT_EQ(a[0], ch);
                break;
            }
            case 4: // Assignment from StringView
            {
                StringView view = b;
                a               = view;
                ASSERT_EQ(a, view);
                break;
            }
            case 5: // operator+
            {
                String combined = a + b;
                for (usize i = 0; i < a.Size(); i++)
                    ASSERT_EQ(combined[i], a[i]);
                for (usize i = 0; i < b.Size(); i++)
                    ASSERT_EQ(combined[i + a.Size()], b[i]);
                break;
            }
            case 6: // operator+=
            {
                usize  oldSize = a.Size();
                String before  = a;

                std::cout << "OldSize: " << oldSize << "\n";
                std::cout << std::setprecision(before.Size()) << before.Size()
                          << "\n";
                std::cout << "A(" << a.Size() << ") => '"
                          << std::setprecision(a.Size()) << a.Raw() << "'\n";
                std::cout << "B(" << b.Size() << ") => '"
                          << std::setprecision(b.Size()) << b.Raw() << "'\n";
                a += b;
                for (usize i = 0; i < oldSize; i++) ASSERT_EQ(a[i], before[i]);
                for (usize i = 0; i < b.Size(); i++)
                    ASSERT_EQ(a[i + oldSize], b[i]);
                break;
            }
            case 7: // swap two strings
            {
                String tmp = a;
                a          = b;
                b          = tmp;
                ASSERT_TRUE(a == tmp || b == tmp);
                break;
            }
            case 8: // compare strings
            {
                bool equal = (a == b);
                for (usize i = 0; i < a.Size() && i < b.Size(); i++)
                {
                    if (a[i] != b[i])
                    {
                        equal = false;
                        break;
                    }
                }
                if (a.Size() != b.Size()) equal = false;
                ASSERT_EQ(equal, (a == b));
                break;
            }
            case 9: // substr test
            {
                if (a.Size() >= 3)
                {
                    usize start = rand() % (a.Size() - 2);
                    usize len   = 1 + rand() % (a.Size() - start);
                    auto  sub   = a.Substr(start, len);
                    for (usize i = 0; i < len; i++)
                        ASSERT_EQ(sub[i], a[start + i]);
                }
                break;
            }
        }

        // Every 1000 iterations, shuffle/reset
        if (iteration % 1000 == 0) pool.clear();
    }
}
TEST(BasicString, String_Trim)
{
    String basic = "   hello world   ";
    ASSERT_EQ(basic.Trim(), "hello world"_sv);

    String left = "   hello";
    ASSERT_EQ(left.Trim(TrimMode::eLeft), "hello");
    ASSERT_EQ(left.Trim(TrimMode::eRight), "   hello");

    String right = "hello   ";
    ASSERT_EQ(right.Trim(TrimMode::eRight), "hello");
    ASSERT_EQ(right.Trim(TrimMode::eLeft), "hello   ");

    String both = "   hello   ";
    ASSERT_EQ(both.Trim(TrimMode::eBoth), "hello");

    String nowhitespace = "hello";
    ASSERT_EQ(nowhitespace.Trim(), "hello");

    String allwhitespace = "     ";
    ASSERT_TRUE(allwhitespace.Trim().Empty());
    ASSERT_TRUE(allwhitespace.Trim(TrimMode::eLeft).Empty());
    ASSERT_TRUE(allwhitespace.Trim(TrimMode::eRight).Empty());

    String empty;
    ASSERT_TRUE(empty.Trim().Empty());

    String sso = "  a  ";
    ASSERT_EQ(sso.Trim(), "a");
}

#if 0
int main()
{
    String_TestConstruction();
    String_TestIteration();
    String_TestAccessors();
    String_TestFind();
    String_TestComparisons();
    String_TestSubstringAndResize();
    String_TestCopyAndSwap();
    String_TestSplit();
    String_TestMove();
    String_TestBasicStringAssignments();
    String_TestBasicStringAddition();
    String_FuzzTestBasicString();
    String_FuzzTestBasicString2();
    String_MegaFuzzTestBasicString();
    String_Trim();

    return 0;
}
#endif
