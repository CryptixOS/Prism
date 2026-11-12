/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Test/TestCase.hpp>

export module Prism.Test.TestSuite;

export import Prism.String.String;
export import Prism.Test.Randomized.RandSource;
export import Prism.Test.TestCase;
export import Prism.Test.TestResult;
export import Prism.Containers.Vector;

export extern "C++"
{
    namespace Prism::Test
    {
        /**
         * Runs the registered test cases. Command line:
         *
         *   test [options] [pattern]
         *     pattern                    only run cases whose name matches
         *                                (glob: '*' and '?')
         *     --list                     list the cases and exit
         *     --tests                    only run tests (also: TESTS_ONLY=1)
         *     --bench                    only run benchmarks
         *     --benchmark_repetitions N  times each benchmark runs (default 1)
         *     --randomized_runs N        runs per randomized test (default 100)
         *     --seed N                   seed for randomized tests
         *
         * The exit status is non-zero when any test failed.
         */
        namespace Suite
        {
            int         Main(int argc, char** argv);

            void        AddCase(const TestCase& testCase);
            void        SetSetup(TestFunction setup);

            TestResult  CurrentResult();
            void        SetCurrentResult(TestResult result);

            bool        IsReportingEnabled();
            void        SetReportingEnabled(bool enabled);

            u64         RandomizedRuns();
            RandSource& CurrentRandSource();

            struct Statistics
            {
                u64 TestsRun         = 0;
                u64 TestsFailed      = 0;
                u64 BenchmarksRun    = 0;
                u64 BenchmarksFailed = 0;
                u64 TestNanos        = 0;
                u64 BenchmarkNanos   = 0;
            };

            bool       RunCase(const TestCase& testCase, u64& nanos);
            bool       RunTest(const TestCase& testCase);
            bool       RunBenchmark(const TestCase& testCase, u64& nanos);
            bool       RunRandomized(const TestCase& testCase);

            TestResult RunWithSource(const TestCase& testCase, RandSource source);
            RandomRun  Shrink(const TestCase& testCase, RandomRun run);
        }; // namespace Suite
    } // namespace Prism::Test
}
