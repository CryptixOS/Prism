/*
 * Created by v1tr10l7 on 04.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Utility.OptionParser;

export import Prism.String.StringView;

export extern "C++"
{
    namespace Prism
    {
        class OptionParser
        {
          public:
            OptionParser(StringView args)
                : m_ArgumentsString(args)
            {
            }

            bool Parse();

          private:
            StringView m_ArgumentsString;
        };
    }; // namespace Prism
}
