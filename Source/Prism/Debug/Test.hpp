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
        std::print(std::cout, "{}\n", std::stacktrace::current());             \
    }
#define PrismTestEq(condition)                                                 \
    {                                                                          \
        if (!(condition))                                                      \
        {                                                                      \
            std::print(std::cout, "`{}` failed\n", #condition);                \
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
