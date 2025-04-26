/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/String/StringView.hpp>

namespace Prism::Algorithm
{
    template <typename C, typename Traits = CharTraits<C>>
    constexpr void ComputeLPSArray(const BasicStringView<C, Traits>& pattern,
                                   Vector<isize>&                    lpsArray)
    {
        usize size = pattern.Size();
        lpsArray.Resize(size);
        lpsArray[0]      = 0;

        isize patternPos = 0;
        for (usize i = 1; i < size; i++)
        {
            while (patternPos > 0 && pattern[patternPos] != pattern[i])
                patternPos = lpsArray[patternPos - 1];
            if (pattern[patternPos] == pattern[i]) ++patternPos;
            lpsArray[i] = patternPos;
        }
    }
    template <typename C, typename Traits = CharTraits<C>>
    constexpr usize FindStringKMP(const BasicStringView<C, Traits>& string,
                                  const BasicStringView<C, Traits>& pattern)
    {
        Vector<isize> lpsArray;
        ComputeLPSArray(pattern, lpsArray);

        isize matchedPos = 0;
        for (usize i = 0; i < string.Size(); i++)
        {
            while (matchedPos > 0 && pattern[matchedPos] != string[i])
                matchedPos = lpsArray[matchedPos - 1];

            if (pattern[matchedPos] == string[i]) ++matchedPos;
            if (matchedPos == static_cast<isize>(pattern.Size()))
                return i - matchedPos + 1;
        }

        return -1;
    }
}; // namespace Prism::Algorithm
