/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/String/String.hpp>
#include <Prism/Utility/Path.hpp>
#include <cassert>
#include <ctime> // time
#include <iomanip>
#include <iostream>
#include <vector>

using namespace Prism;
using namespace Prism::Literals;

void String_TestConstruction()
{
    String empty;
    assert(empty.Empty());
    assert(empty.Size() == 0);

    String repeated(5, 'x');
    assert(repeated == "xxxxx"_s);

    const char* raw = "hello";
    String      fromRaw(raw);
    assert(fromRaw == "hello"_s);

    String fromView = "world"_s;
    assert(fromView == "world"_s);
}

void String_TestIteration()
{
    String      s = "abc"_s;

    std::string built;
    for (char c : s) built += c;

    assert(built == "abc");
}

void String_TestAccessors()
{
    String s = "hello"_s;

    assert(s.At(0) == 'h');
    assert(s[1] == 'e');
    assert(s.Front() == 'h');
    assert(s.Back() == 'o');
}

void String_TestFind()
{
    String s = "hello world"_s;

    assert(s.Find("world") == 6);
    assert(s.Find('o') == 4);
    assert(s.RFind('o') == 7);
    s = "hello world"_s;

    // Find
    assert(s.Find("hello") == 0);
    assert(s.Find("world") == 6);
    assert(s.Find("o") == 4);
    assert(s.Find("notfound") == String::NPos);
    assert(s.Find('l') == 2);

    // RFind
    assert(s.RFind("hello") == 0);
    assert(s.RFind("world") == 6);
    assert(s.RFind("o") == 7);
    assert(s.RFind('l') == 9);
    assert(s.RFind("notfound") == String::NPos);

    // FindFirstOf
    assert(s.FindFirstOf("aeiou"_s) == 1); // 'e'
    assert(s.FindFirstOf("xyz"_s) == String::NPos);
    assert(s.FindFirstOf('o') == 4);

    // FindFirstNotOf
    assert(s.FindFirstNotOf("hel"_s) == 4); // 'o'
    assert(s.FindFirstNotOf("hello wrd"_s)
           == String::NPos); // all characters matched
    assert(s.FindFirstNotOf('h') == 1);

    // FindLastOf
    assert(s.FindLastOf("ol") == 9); // last 'l'
    assert(s.FindLastOf("xyz") == String::NPos);
    assert(s.FindLastOf('l') == 9);

    // FindLastNotOf
    // assert(s.FindLastNotOf("d"_s) == 9); // 'l'
    // assert(s.FindLastNotOf("helo wrd"_s)
    //       == String::NPos); // all characters matched
    // assert(s.FindLastNotOf('d') == 9);
}

void String_TestComparisons()
{
    String a = "abc"_s;
    String b = "abc"_s;
    String c = "abd"_s;

    assert(a == b);
    assert(a != c);
    assert((a <=> c) == std::strong_ordering::less);
}

void String_TestSubstringAndResize()
{
    String s   = "abcdef"_s;

    String sub = s.Substr(1, 3);
    assert(sub == "bcd"_s);

    s.Resize(3);
    printf("s1: %s\n", s.Raw());
    printf("s2: %s\n", "abc"_s.Raw());
    assert(s == "abc"_s);

    s.Resize(5, 'x');
    assert(s == "abcxx"_s);
}

void String_TestCopyAndSwap()
{
    String a = "abc"_s;
    String b = "xyz"_s;

    a.Swap(b);
    assert(a == "xyz"_s);
    assert(b == "abc"_s);

    a = b;
    assert(a == "abc"_s);
}

Path String_TestPath(PathView path)
{
    auto segments = path.Split();
    auto strings  = StringView(path).Split('/');
    for (usize i = 0; i < segments.Size(); i++)
    {
        String     segment = segments[i];
        StringView string  = strings[i];
        std::cout << "VFS: '" << std::setprecision(segment.Size())
                  << segment.Raw() << "' == '"
                  << std::setprecision(string.Size()) << string.Raw() << "'"
                  << (segment == string) << "\n";

        if (i == segments.Size() - 1) return Path(segment);
    }

    return !segments.Empty() ? segments.Front() : ":(";
}
void String_TestSplit()
{
    PathView path         = "/usr/sbin/string/someother/anyway/yo/hello.txt"_sv;
    auto     newPath      = String_TestPath(path);
    auto     result       = String_TestPath(path);
    auto     concatenated = String(result.View());
    concatenated += String(newPath.View());
    String_TestPath(concatenated);
}
void String_TestMove()
{
    String str   = "Hello";
    auto   other = std::move(str);
    assert(str.Empty());
    assert(other.Size() == 5);

    String next(std::move(other));
    assert(other.Empty());
    assert(next.Size() == 5);
}

void String_TestBasicStringAssignments()
{
    using String = BasicString<char>;

    // Test copy assignment
    {
        String a = "Hello";
        String b;
        b = a;
        assert(b == "Hello");
        assert(a == "Hello");
        assert(b.Size() == 5);
    }

    // Test move assignment
    {
        String a = "World";
        String b;
        b = std::move(a);
        assert(b == "World");
        // We can't assume `a` still contains valid data after move, only that
        // it's safe to destroy.
        assert(b.Size() == 5);
    }

    // Test assignment from const char*
    {
        String a;
        a = "Test123";
        assert(a == "Test123");
        assert(a.Size() == 7);
    }

    // Test assignment from a single character
    {
        String a;
        a = 'Z';
        assert(a == "Z");
        assert(a.Size() == 1);
    }

    // Test assignment from StringView-like object
    {
        String     a = "initial";
        StringView view("stringview");

        a = view;
        assert(a == "stringview");
        assert(a.Size() == 10);
    }
}

void String_TestBasicStringAddition()
{
    using String = BasicString<char>;

    // Assuming you have operator+(const BasicString&, const BasicString&)
    {
        String a = "Hello";
        String b = "World";
        String c = a + b;

        assert(a == "Hello"); // Must stay unchanged
        assert(b == "World"); // Must stay unchanged
        assert(c == "HelloWorld");
        assert(c.Size() == 10);
    }

    // Addition with const char* (if supported)
    {
        String a = "Foo";
        String b = a + "Bar";

        assert(a == "Foo"); // a must stay unchanged
        assert(b == "FooBar");
        assert(b.Size() == 6);
    }

    // Addition with empty string
    {
        String a = "NonEmpty";
        String b;
        String c = a + b;

        assert(c == "NonEmpty");
        assert(c.Size() == 8);
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
void String_FuzzTestBasicString()
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
        assert(copy == a);
        assert(copy.Size() == a.Size());

        // Move assignment
        String move = std::move(b);
        assert(move.Size() == len2); // b was random of size len2

        // Assignment from const char*
        const char* literal = "LiteralString";
        a                   = literal;
        assert(a == "LiteralString");
        assert(a.Size() == strlen(literal));

        // Assignment from single char
        b = 'X';
        assert(b == "X");
        assert(b.Size() == 1);

        // Assignment from StringView
        StringView view("ViewContent");
        a = view;
        assert(a == "ViewContent");
        assert(a.Size() == strlen("ViewContent"));

        // Operator+
        String combined = a + b;
        assert(combined == "ViewContentX");
        assert(combined.Size() == strlen("ViewContentX"));
    }
}

void String_FuzzTestBasicString2()
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
        assert(copy == a);
        assert(copy.Size() == a.Size());

        // --- Self-copy assignment ---
        a = a;
        assert(a == copy);

        // --- Move assignment ---
        String move = std::move(b);
        assert(move.Size() == len2); // b was random of size len2

        // --- Assignment from const char* ---
        const char* literal = "LiteralString";
        a                   = literal;
        assert(a == "LiteralString");
        assert(a.Size() == strlen(literal));

        // --- Assignment from single char ---
        b = 'X';
        assert(b == "X");
        assert(b.Size() == 1);

        // --- Assignment from StringView ---
        StringView view("ViewContent");
        a = view;
        assert(a == "ViewContent");
        assert(a.Size() == strlen("ViewContent"));

        // --- Operator+ ---
        String combined = a + b;
        assert(combined == "ViewContentX");
        assert(combined.Size() == strlen("ViewContentX"));

        // --- Operator+= ---
        a += b;
        assert(a == "ViewContentX");
        assert(a.Size() == strlen("ViewContentX"));

        // --- Assignment with empty string ---
        a = "";
        assert(a.Empty());
        assert(a.Size() == 0);

        // --- Concatenation with empty ---
        String c         = String_RandomString(rand() % 20);
        String d         = "";
        String combined2 = c + d;
        assert(combined2 == c);

        combined2 = d + c;
        assert(combined2 == c);

        // --- Fuzz: Random characters append ---
        usize  randomPushCount = rand() % 50;
        String fuzzString;
        fuzzString.Reserve(randomPushCount);
        for (usize j = 0; j < randomPushCount; j++)
        {
            char randomChar = String_RandomChar();
            fuzzString += randomChar;
            assert(fuzzString.Size() == j + 1);
            assert(fuzzString.Back() == randomChar);
        }
    }
}

void String_MegaFuzzTestBasicString()
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
                assert(copy == a);
                a = copy;
                assert(a == copy);
                break;
            }
            case 1: // Move assign
            {
                String tmp   = b;
                String moved = std::move(tmp);
                assert(moved == b);
                break;
            }
            case 2: // Assignment from const char*
            {
                const char* literals[]
                    = {"Test", "Mega", "Fuzz", "", "String!"};
                const char* lit = literals[rand() % 5];
                a               = lit;
                assert(a == lit);
                break;
            }
            case 3: // Assignment from char
            {
                char ch = String_RandomChar();
                a       = ch;
                assert(a.Size() == 1);
                assert(a[0] == ch);
                break;
            }
            case 4: // Assignment from StringView
            {
                StringView view = b;
                a               = view;
                assert(a == view);
                break;
            }
            case 5: // operator+
            {
                String combined = a + b;
                for (usize i = 0; i < a.Size(); i++)
                    assert(combined[i] == a[i]);
                for (usize i = 0; i < b.Size(); i++)
                    assert(combined[i + a.Size()] == b[i]);
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
                for (usize i = 0; i < oldSize; i++) assert(a[i] == before[i]);
                for (usize i = 0; i < b.Size(); i++)
                    assert(a[i + oldSize] == b[i]);
                break;
            }
            case 7: // swap two strings
            {
                String tmp = a;
                a          = b;
                b          = tmp;
                assert(a == tmp || b == tmp);
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
                assert(equal == (a == b));
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
                        assert(sub[i] == a[start + i]);
                }
                break;
            }
        }

        // Every 1000 iterations, shuffle/reset
        if (iteration % 1000 == 0) pool.clear();
    }
}
void String_Trim()
{
    String basic = "   hello world   ";
    assert(basic.Trim() == "hello world"_sv);

    String left = "   hello";
    assert(left.Trim(TrimMode::eLeft) == "hello");
    assert(left.Trim(TrimMode::eRight) == "   hello");

    String right = "hello   ";
    assert(right.Trim(TrimMode::eRight) == "hello");
    assert(right.Trim(TrimMode::eLeft) == "hello   ");

    String both = "   hello   ";
    assert(both.Trim(TrimMode::eBoth) == "hello");

    String nowhitespace = "hello";
    assert(nowhitespace.Trim() == "hello");

    String allwhitespace = "     ";
    assert(allwhitespace.Trim().Empty());
    assert(allwhitespace.Trim(TrimMode::eLeft).Empty());
    assert(allwhitespace.Trim(TrimMode::eRight).Empty());

    String empty;
    assert(empty.Trim().Empty());

    String sso = "  a  ";
    assert(sso.Trim() == "a");
}

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
