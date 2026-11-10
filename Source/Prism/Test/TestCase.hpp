/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/String/StringView.hpp>

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
        {
            return m_Kind == TestCaseKind::eBenchmark;
        }
        constexpr bool IsRandomized() const
        {
            return m_Kind == TestCaseKind::eRandomized;
        }

      private:
        StringView   m_Name;
        TestFunction m_Function;
        TestCaseKind m_Kind;
    };

    void AddTestCaseToSuite(const TestCase& testCase);
    void SetSuiteSetupFunction(TestFunction setup);
} // namespace Prism::Test

#define PRISM_TEST_DEFINE_CASE(name, kind)                                     \
    static void PrismTestCase_##name();                                        \
    static struct PrismTestCaseRegistrar_##name                                \
    {                                                                          \
        PrismTestCaseRegistrar_##name()                                        \
        {                                                                      \
            ::Prism::Test::AddTestCaseToSuite(::Prism::Test::TestCase(         \
                #name, PrismTestCase_##name, kind));                           \
        }                                                                      \
    } PrismTestCaseRegistrarInstance_##name;                                   \
    static void PrismTestCase_##name()

/// Defines a test case: `TEST_CASE(vector_push_back) { EXPECT(...); }`.
#define TEST_CASE(name)                                                        \
    PRISM_TEST_DEFINE_CASE(name, ::Prism::Test::TestCaseKind::eTest)

/// Defines a benchmark, run with --bench (or by default, unless --tests or
/// TESTS_ONLY is given) and timed over --benchmark_repetitions runs.
#define BENCHMARK_CASE(name)                                                   \
    PRISM_TEST_DEFINE_CASE(name, ::Prism::Test::TestCaseKind::eBenchmark)

/// Defines a property-based test: inputs come from GEN(), the test is run
/// --randomized_runs times and failing inputs are shrunk to a minimal
/// counterexample.
#define RANDOMIZED_TEST_CASE(name)                                             \
    PRISM_TEST_DEFINE_CASE(name, ::Prism::Test::TestCaseKind::eRandomized)

/// Code run before every test case of the executable.
#define TEST_SETUP                                                             \
    static void PrismTestSetup();                                              \
    static struct PrismTestSetupRegistrar                                      \
    {                                                                          \
        PrismTestSetupRegistrar()                                              \
        {                                                                      \
            ::Prism::Test::SetSuiteSetupFunction(PrismTestSetup);              \
        }                                                                      \
    } PrismTestSetupRegistrarInstance;                                         \
    static void PrismTestSetup()
