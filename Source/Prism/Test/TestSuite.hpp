/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/String/String.hpp>
#include <Prism/Test/Randomized/RandSource.hpp>
#include <Prism/Test/TestCase.hpp>
#include <Prism/Test/TestResult.hpp>

namespace Prism::Test
{
    /**
     * Runs the registered test cases. Command line (same as Serenity's
     * LibTest, plus --seed and --help):
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
    class TestSuite
    {
      public:
        static TestSuite& The();

        int               Main(int argc, char** argv);

        void              AddCase(const TestCase& testCase);
        void              SetSetup(TestFunction setup) { m_Setup = setup; }

        TestResult        CurrentResult() const { return m_CurrentResult; }
        void              SetCurrentResult(TestResult result);

        bool              IsReportingEnabled() const { return m_Reporting; }
        void SetReportingEnabled(bool enabled) { m_Reporting = enabled; }

        u64  RandomizedRuns() const { return m_RandomizedRuns; }
        RandSource& CurrentRandSource() { return m_RandSource; }

      private:
        TestSuite() = default;

        struct Statistics
        {
            u64 TestsRun        = 0;
            u64 TestsFailed     = 0;
            u64 BenchmarksRun   = 0;
            u64 BenchmarksFailed = 0;
            u64 TestNanos       = 0;
            u64 BenchmarkNanos  = 0;
        };

        Vector<TestCase> m_Cases;
        TestFunction     m_Setup                = nullptr;
        TestResult       m_CurrentResult        = TestResult::eNotRun;
        bool             m_Reporting            = true;
        u64              m_RandomizedRuns       = 100;
        u64              m_BenchmarkRepetitions = 1;
        u64              m_Seed                 = 0;
        Random           m_Random;
        RandSource       m_RandSource;

        bool             RunCase(const TestCase& testCase, u64& nanos);
        bool             RunTest(const TestCase& testCase);
        bool             RunBenchmark(const TestCase& testCase, u64& nanos);
        bool             RunRandomized(const TestCase& testCase);

        TestResult       RunWithSource(const TestCase& testCase,
                                       RandSource       source);
        RandomRun        Shrink(const TestCase& testCase, RandomRun run);
    };
} // namespace Prism::Test
