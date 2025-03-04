/*
 * Created by v1tr10l7 on 04.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/StringView.hpp>

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
