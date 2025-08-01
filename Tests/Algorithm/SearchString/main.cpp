/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Algorithm/SearchString.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/String/String.hpp>

#include <iomanip>
#include <iostream>

using namespace Prism;
using namespace Prism::Literals;

void KPM_Search_TestComputeLPS_Array()
{
    Vector<isize> expected = {0, 0, 1, 2, 0};
    String        pattern  = "ababd";
    Vector<isize> lps;
    Algorithm::ComputeLPSArray(pattern.View(), lps);

    assert(lps == expected);
}
void test_KMPearch_SingleMatch()
{
    String        string          = "ababcabcabababd";
    String        pattern         = "ababd";

    Vector<isize> expectedIndices = {10};
    Vector<isize> foundIndices;
    auto          KMPearchText = [&](const String& text, const String& pattern)
    {
        isize         textLength    = text.Size();
        isize         patternLength = pattern.Size();
        Vector<isize> lps;
        Algorithm::ComputeLPSArray(pattern.View(), lps);

        isize textPos    = 0;
        isize patternPos = 0;

        while (textPos < textLength)
        {
            if (pattern[patternPos] == text[textPos]) ++patternPos, ++textPos;
            if (patternPos == patternLength)
            {
                foundIndices.PushBack(patternPos - patternPos);
                patternPos = lps[patternPos - 1];
                continue;
            }

            if (textPos < textLength && pattern[patternPos] != text[textPos])
            {
                patternPos = (patternPos != 0) ? lps[patternPos - 1] : 0;
                if (patternPos == 0) ++textPos;
            }
        }
    };

    KMPearchText(string, pattern);
    assert(foundIndices == expectedIndices);
}

isize KMP_Search(const String& text, const String& pattern)
{
    isize pos = Algorithm::FindStringKMP(text.View(), pattern.View());
    std::cout << "Searching for '" << std::setprecision(pattern.Size())
              << pattern.Raw() << "' in '" << std::setprecision(text.Size())
              << text.Raw() << "'...\n";
    std::cout << "Returned index => " << pos << "\n";

    return pos;
}
isize KMP_Search(const String& text, const char ch)
{
    char       buf[2] = {ch, '\0'};
    StringView pattern(buf, 1);
    return KMP_Search(text.View(), pattern);
}

void KPM_Search_TestFind()
{
    String s = "hello world"_s;

    assert(KMP_Search(s, "world") == 6);
    assert(KMP_Search(s, 'o') == 4);
    s = "hello world"_s;

    // Find
    assert(KMP_Search(s, "hello") == 0);
    assert(KMP_Search(s, "world") == 6);
    assert(KMP_Search(s, "o") == 4);
    assert(KMP_Search(s, "notfound") < 0);
    assert(KMP_Search(s, 'l') == 2);

    // TODO(v1tr10l7): Implement RFind, FindFirstOf, FindFirstNotOf, FindLastOf
}

int main()
{
    KPM_Search_TestComputeLPS_Array();
    KPM_Search_TestFind();

    return EXIT_SUCCESS;
}
