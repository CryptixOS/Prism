/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Types.hpp>

// Macros only: the code lives in module Prism.Test.TestCase, which
// files using these macros import too.

#define PRISM_TEST_DEFINE_CASE(name, kind)                                     \
    static void PrismTestCase_##name();                                        \
    static struct PrismTestCaseRegistrar_##name                                \
    {                                                                          \
        PrismTestCaseRegistrar_##name()                                        \
        {                                                                      \
            ::Prism::Test::Suite::AddCase(                                     \
                ::Prism::Test::TestCase(#name, PrismTestCase_##name, kind));   \
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
        { ::Prism::Test::Suite::SetSetup(PrismTestSetup); }                    \
    } PrismTestSetupRegistrarInstance;                                         \
    static void PrismTestSetup()
