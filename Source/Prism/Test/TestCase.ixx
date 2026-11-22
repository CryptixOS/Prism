/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>

#include <Prism/Test/TestCase.hpp>

export module Prism.Test.TestCase;

export import Prism.String.StringView;

export extern "C++"
{
    namespace Prism::Test
    {
        using TestFunction = void (*)();

        enum class TestCaseKind : u8
        {
            eTest,
            eBenchmark,
            eRandomized,
        };

        class TestCase
        {
          public:
            constexpr TestCase(StringView name, TestFunction function,
                               TestCaseKind kind)
                : m_Name(name)
                , m_Function(function)
                , m_Kind(kind)
            {
            }

            constexpr StringView   Name() const { return m_Name; }
            constexpr TestFunction Function() const { return m_Function; }
            constexpr TestCaseKind Kind() const { return m_Kind; }
            constexpr bool         IsBenchmark() const
            { return m_Kind == TestCaseKind::eBenchmark; }
            constexpr bool IsRandomized() const
            { return m_Kind == TestCaseKind::eRandomized; }

          private:
            StringView   m_Name;
            TestFunction m_Function;
            TestCaseKind m_Kind;
        };
    } // namespace Prism::Test
}
