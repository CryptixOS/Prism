/*
 * Created by v1tr10l7 on 04.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <format>
#include <iostream>
#include <stacktrace>

#define DumpTrace()                                                            \
    {                                                                          \
        std::cout << std::stacktrace::current() << std::endl;                  \
    }
#define PrismTestEq(condition)                                                 \
    {                                                                          \
        if (!(condition))                                                      \
        {                                                                      \
            std::cout << "`" << #condition << "` failed\n";                    \
            DumpTrace();                                                       \
            std::terminate();                                                  \
        }                                                                      \
    }

namespace Prism
{
    class TestCase
    {
      public:
        TestCase();

      private:
    };

}; // namespace Prism
