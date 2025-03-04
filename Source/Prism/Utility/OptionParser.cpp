/*
 * Created by v1tr10l7 on 04.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Utility/OptionParser.hpp>

#include <cctype>

namespace Prism
{
    bool OptionParser::Parse()
    {
        usize argStart   = 0;
        usize argEnd     = 0;

        usize valueStart = 0;
        usize valueEnd   = 0;
        enum
        {
            eArg   = 1,
            eValue = 2,
        };
        usize whatParsing = 0;

        for (usize i = 0; char c : m_ArgumentsString)
        {
            if (c == '-')
            {
                whatParsing = eArg;
                argStart    = i + 1;
            }

            if (!std::isalpha(*(&c + 1)))
                ;
            if (c == '=')
            {
                whatParsing = eValue;
                valueStart  = i + 1;
            }
        }
    }

}; // namespace Prism
