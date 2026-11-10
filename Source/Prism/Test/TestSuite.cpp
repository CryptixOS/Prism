/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

namespace Prism::Test
{
    // Free functions used by the macros.

    TestResult CurrentTestResult() { return TestSuite::The().CurrentResult(); }
    void       SetCurrentTestResult(TestResult result)
    {
        TestSuite::The().SetCurrentResult(result);
    }
    void CurrentTestCaseDidFail() { SetCurrentTestResult(TestResult::eFailed); }
    bool CurrentTestCaseFailed()
    {
        return CurrentTestResult() == TestResult::eFailed;
    }
    bool IsReportingEnabled() { return TestSuite::The().IsReportingEnabled(); }
    void EnableReporting() { TestSuite::The().SetReportingEnabled(true); }
    void DisableReporting() { TestSuite::The().SetReportingEnabled(false); }
    u64  RandomizedRuns() { return TestSuite::The().RandomizedRuns(); }
    RandSource& CurrentRandSource() { return TestSuite::The().CurrentRandSource(); }

    void AddTestCaseToSuite(const TestCase& testCase)
    {
        TestSuite::The().AddCase(testCase);
    }
    void SetSuiteSetupFunction(TestFunction setup)
    {
        TestSuite::The().SetSetup(setup);
    }

    void WriteOutput(StringView text)
    {
        const char* data = text.Raw();
        usize       size = text.Size();
        while (size > 0)
        {
            auto written = ::write(STDERR_FILENO, data, size);
            if (written <= 0) return;
            data += written;
            size -= static_cast<usize>(written);
        }
    }
    bool OutputIsTerminal()
    {
        static int s_IsTerminal = ::isatty(STDERR_FILENO);
        return s_IsTerminal;
    }

    namespace Details
    {
        StringView FailTag()
        {
            return OutputIsTerminal() ? "\033[31;1mFAIL\033[0m" : "FAIL";
        }
        static StringView PassTag()
        {
            return OutputIsTerminal() ? "\033[32;1mPASS\033[0m" : "PASS";
        }

        String FormatDouble(double value)
        {
            char buffer[64];
            int  length = snprintf(buffer, sizeof(buffer), "%.17g", value);
            return String(buffer, length > 0 ? static_cast<usize>(length) : 0);
        }

        void ReportApproximateFailure(const char* lhsText, const char* rhsText,
                                      double lhs, double rhs,
                                      const char* file, u32 line)
        {
            if (IsReportingEnabled())
                Warnln("{}: {}:{}: EXPECT_APPROXIMATE({}, {}) failed with "
                       "lhs={}, rhs={}, (lhs-rhs)={}",
                       FailTag(), file, line, lhsText, rhsText,
                       FormatDouble(lhs), FormatDouble(rhs),
                       FormatDouble(lhs - rhs));
            CurrentTestCaseDidFail();
        }

        static u64 NowNanos()
        {
            timespec ts{};
            clock_gettime(CLOCK_MONOTONIC, &ts);
            return static_cast<u64>(ts.tv_sec) * 1'000'000'000ull
                 + static_cast<u64>(ts.tv_nsec);
        }
        static u64 ToMillis(u64 nanos) { return nanos / 1'000'000ull; }

        /// Glob match supporting '*' and '?'.
        static bool Matches(StringView text, StringView pattern)
        {
            usize t = 0, p = 0, star = StringView::NPos, mark = 0;
            while (t < text.Size())
            {
                if (p < pattern.Size()
                    && (pattern[p] == '?' || pattern[p] == text[t]))
                {
                    ++t;
                    ++p;
                }
                else if (p < pattern.Size() && pattern[p] == '*')
                {
                    star = p++;
                    mark = t;
                }
                else if (star != StringView::NPos)
                {
                    p = star + 1;
                    t = ++mark;
                }
                else return false;
            }
            while (p < pattern.Size() && pattern[p] == '*') ++p;
            return p == pattern.Size();
        }

        static bool ParseNumber(const char* text, u64& out)
        {
            if (!text || !*text) return false;
            char* end = nullptr;
            out       = strtoull(text, &end, 0);
            return end && *end == '\0';
        }
    } // namespace Details

    using namespace Details;

    TestSuite& TestSuite::The()
    {
        static TestSuite s_Instance;
        return s_Instance;
    }

    void TestSuite::AddCase(const TestCase& testCase)
    {
        m_Cases.PushBack(testCase);
    }

    void TestSuite::SetCurrentResult(TestResult result)
    {
        // A failure sticks, unless the input turned out to be invalid.
        if (m_CurrentResult == TestResult::eFailed
            && result == TestResult::ePassed)
            return;
        m_CurrentResult = result;
    }

    static void PrintUsage(const char* program)
    {
        Warnln("Usage: {} [options] [pattern]", program);
        Warnln("  pattern                    only run matching cases (glob)");
        Warnln("  --list                     list the available cases");
        Warnln("  --tests                    only run tests (or TESTS_ONLY=1)");
        Warnln("  --bench                    only run benchmarks");
        Warnln("  --benchmark_repetitions N  times each benchmark runs "
               "(default 1)");
        Warnln("  --randomized_runs N        runs per RANDOMIZED_TEST_CASE "
               "(default 100)");
        Warnln("  --seed N                   seed for randomized tests");
        Warnln("  --help                     show this help");
    }

    int TestSuite::Main(int argc, char** argv)
    {
        bool       testsOnly      = getenv("TESTS_ONLY") != nullptr;
        bool       benchmarksOnly = false;
        bool       listCases      = false;
        bool       seedGiven      = false;
        StringView pattern        = "*";
        const char* program       = argc > 0 ? argv[0] : "test";

        for (int i = 1; i < argc; ++i)
        {
            StringView arg = argv[i];
            auto       numberOption
                = [&](StringView name, u64& out) -> int
            {
                // Accepts "--name N" and "--name=N"; returns 0 if `arg`
                // isn't this option, -1 on a bad value.
                if (arg == name)
                {
                    if (i + 1 >= argc || !ParseNumber(argv[i + 1], out))
                        return -1;
                    ++i;
                    return 1;
                }
                if (arg.Size() > name.Size() && arg.StartsWith(name)
                    && arg[name.Size()] == '=')
                    return ParseNumber(argv[i] + name.Size() + 1, out) ? 1
                                                                       : -1;
                return 0;
            };

            int result = 0;
            if (arg == "--help" || arg == "-h")
            {
                PrintUsage(program);
                return 0;
            }
            else if (arg == "--list") listCases = true;
            else if (arg == "--tests") testsOnly = true;
            else if (arg == "--bench") benchmarksOnly = true;
            else if ((result = numberOption("--benchmark_repetitions",
                                            m_BenchmarkRepetitions))
                     != 0)
                ;
            else if ((result = numberOption("--randomized_runs",
                                            m_RandomizedRuns))
                     != 0)
                ;
            else if ((result = numberOption("--seed", m_Seed)) != 0)
                seedGiven = result > 0;
            else if (arg.Size() > 0 && arg[0] == '-')
            {
                Warnln("Unknown option '{}'", arg);
                PrintUsage(program);
                return 1;
            }
            else pattern = arg;

            if (result < 0)
            {
                Warnln("Invalid value for '{}'", arg);
                return 1;
            }
        }

        if (listCases)
        {
            for (const auto& testCase : m_Cases)
                Warnln("    {}: {}",
                       testCase.IsBenchmark()    ? "benchmark"
                       : testCase.IsRandomized() ? "randomized"
                                                 : "test",
                       testCase.Name());
            return 0;
        }

        if (!seedGiven) m_Seed = NowNanos() ^ static_cast<u64>(getpid());
        m_Random.Seed(m_Seed);

        Vector<const TestCase*> selected;
        for (const auto& testCase : m_Cases)
        {
            if (testsOnly && testCase.IsBenchmark()) continue;
            if (benchmarksOnly && !testCase.IsBenchmark()) continue;
            if (pattern != "*" && !Matches(testCase.Name(), pattern)) continue;
            selected.PushBack(&testCase);
        }

        Warnln("Running {} cases out of {}.", selected.Size(), m_Cases.Size());

        Statistics stats;
        u64        start = NowNanos();
        for (auto* testCase : selected)
        {
            u64  nanos  = 0;
            bool passed = RunCase(*testCase, nanos);
            if (testCase->IsBenchmark())
            {
                ++stats.BenchmarksRun;
                stats.BenchmarksFailed += !passed;
                stats.BenchmarkNanos += nanos;
            }
            else
            {
                ++stats.TestsRun;
                stats.TestsFailed += !passed;
                stats.TestNanos += nanos;
            }
        }
        u64 total = NowNanos() - start;
        u64 other = total - stats.TestNanos - stats.BenchmarkNanos;

        Warnln("Finished {} tests and {} benchmarks in {}ms ({}ms tests, {}ms "
               "benchmarks, {}ms other).",
               stats.TestsRun, stats.BenchmarksRun, ToMillis(total),
               ToMillis(stats.TestNanos), ToMillis(stats.BenchmarkNanos),
               ToMillis(other));

        u64 failed = stats.TestsFailed + stats.BenchmarksFailed;
        if (failed == 0) Warnln("All tests passed!");
        else
        {
            if (stats.TestsRun > 0)
                Warnln("Out of {} tests, {} passed and {} failed.",
                       stats.TestsRun, stats.TestsRun - stats.TestsFailed,
                       stats.TestsFailed);
            if (stats.BenchmarksRun > 0)
                Warnln("Out of {} benchmarks, {} passed and {} failed.",
                       stats.BenchmarksRun,
                       stats.BenchmarksRun - stats.BenchmarksFailed,
                       stats.BenchmarksFailed);
        }
        return failed > 0 ? 1 : 0;
    }

    bool TestSuite::RunCase(const TestCase& testCase, u64& nanos)
    {
        Warnln("START  Running {} '{}'",
               testCase.IsBenchmark()    ? "benchmark"
               : testCase.IsRandomized() ? "randomized test"
                                         : "test",
               testCase.Name());

        u64  start  = NowNanos();
        bool passed = testCase.IsBenchmark()    ? RunBenchmark(testCase, nanos)
                    : testCase.IsRandomized() ? RunRandomized(testCase)
                                                : RunTest(testCase);
        if (!testCase.IsBenchmark()) nanos = NowNanos() - start;

        if (passed)
            Warnln("{}   {} ({}ms)", PassTag(), testCase.Name(), ToMillis(nanos));
        else
            Warnln("{}   {} ({}ms)", FailTag(), testCase.Name(), ToMillis(nanos));
        return passed;
    }

    bool TestSuite::RunTest(const TestCase& testCase)
    {
        m_Reporting     = true;
        m_CurrentResult = TestResult::ePassed;
        if (m_Setup) m_Setup();
        testCase.Function()();
        return m_CurrentResult != TestResult::eFailed;
    }

    bool TestSuite::RunBenchmark(const TestCase& testCase, u64& nanos)
    {
        u64  repetitions = m_BenchmarkRepetitions > 0 ? m_BenchmarkRepetitions : 1;
        u64  best = ~0ull, worst = 0;
        bool passed = true;
        nanos       = 0;
        for (u64 i = 0; i < repetitions; ++i)
        {
            u64 start = NowNanos();
            passed &= RunTest(testCase);
            u64 elapsed = NowNanos() - start;
            nanos += elapsed;
            if (elapsed < best) best = elapsed;
            if (elapsed > worst) worst = elapsed;
        }
        if (repetitions > 1)
            Warnln("       {} repetitions: avg {}us, min {}us, max {}us",
                   repetitions, nanos / repetitions / 1000, best / 1000,
                   worst / 1000);
        return passed;
    }

    TestResult TestSuite::RunWithSource(const TestCase& testCase,
                                        RandSource       source)
    {
        m_RandSource    = Move(source);
        m_CurrentResult = TestResult::ePassed;
        if (m_Setup) m_Setup();
        testCase.Function()();
        return m_CurrentResult;
    }

    bool TestSuite::RunRandomized(const TestCase& testCase)
    {
        // Give up when this many inputs in a row are rejected.
        constexpr u64 MaxRejectionsPerRun = 15;

        u64 passedRuns = 0, rejectedInARow = 0;
        while (passedRuns < m_RandomizedRuns)
        {
            m_Reporting = false;
            auto result = RunWithSource(testCase, RandSource::Live(m_Random));
            if (result == TestResult::eRejected
                || result == TestResult::eOverrun)
            {
                if (++rejectedInARow >= MaxRejectionsPerRun)
                {
                    m_Reporting = true;
                    Warnln("{}: gave up after {} rejected inputs in a row "
                           "({} runs passed)",
                           FailTag(), rejectedInARow, passedRuns);
                    m_CurrentResult = TestResult::eHitLimit;
                    return false;
                }
                continue;
            }
            rejectedInARow = 0;

            if (result == TestResult::eFailed)
            {
                RandomRun counterexample
                    = Shrink(testCase, m_RandSource.Run());

                // Replay the minimal counterexample with reporting on, so
                // GEN() prints the values and the expectations their errors.
                m_Reporting = true;
                Warnln("Failed after {} successful runs; minimal "
                       "counterexample (seed {}):",
                       passedRuns, m_Seed);
                RunWithSource(testCase, RandSource::Recorded(counterexample));
                m_CurrentResult = TestResult::eFailed;
                return false;
            }
            ++passedRuns;
        }
        m_Reporting = true;
        return true;
    }

    RandomRun TestSuite::Shrink(const TestCase& testCase, RandomRun run)
    {
        constexpr u64 MaxAttempts = 10'000;
        u64           attempts    = 0;

        auto          fails       = [&](const RandomRun& candidate)
        {
            ++attempts;
            return RunWithSource(testCase, RandSource::Recorded(candidate))
                == TestResult::eFailed;
        };
        auto tryCandidate = [&](const RandomRun& candidate)
        {
            if (attempts >= MaxAttempts
                || !candidate.IsShortlexSmallerThan(run) || !fails(candidate))
                return false;
            run = candidate;
            return true;
        };

        bool improved = true;
        while (improved && attempts < MaxAttempts)
        {
            improved = false;

            for (usize chunk : {8, 4, 2, 1})
            {
                for (usize start = 0; start + chunk <= run.Size();)
                {
                    bool changed = false;

                    // Delete the chunk (shortens collections).
                    changed = tryCandidate(run.WithoutChunk(start, chunk));

                    // Delete it and decrement the value before it (often a
                    // length or a "keep going" choice).
                    if (!changed && start > 0 && run[start - 1] > 0)
                    {
                        auto candidate = run.WithoutChunk(start, chunk);
                        --candidate[start - 1];
                        changed = tryCandidate(candidate);
                    }

                    // Zero the chunk.
                    if (!changed)
                    {
                        auto candidate = run;
                        bool nonZero   = false;
                        for (usize i = start; i < start + chunk; ++i)
                        {
                            nonZero |= candidate[i] != 0;
                            candidate[i] = 0;
                        }
                        if (nonZero) changed = tryCandidate(candidate);
                    }

                    // Sort the chunk.
                    if (!changed && chunk > 1)
                    {
                        auto candidate = run;
                        for (usize i = start + 1; i < start + chunk; ++i)
                            for (usize j = i; j > start
                                              && candidate[j - 1] > candidate[j];
                                 --j)
                            {
                                u64 tmp          = candidate[j];
                                candidate[j]     = candidate[j - 1];
                                candidate[j - 1] = tmp;
                            }
                        changed = tryCandidate(candidate);
                    }

                    if (changed) improved = true;
                    else ++start;
                }
            }

            // Minimize every value with a binary search.
            for (usize i = 0; i < run.Size(); ++i)
            {
                if (run[i] == 0) continue;
                auto candidate = run;
                candidate[i]   = 0;
                if (tryCandidate(candidate))
                {
                    improved = true;
                    continue;
                }

                u64 low = 0, high = run[i];
                while (low + 1 < high && attempts < MaxAttempts)
                {
                    u64 middle   = low + (high - low) / 2;
                    candidate    = run;
                    candidate[i] = middle;
                    if (tryCandidate(candidate))
                    {
                        high     = middle;
                        improved = true;
                    }
                    else low = middle;
                }
            }

            // Move value from an earlier choice to a later one (e.g. from
            // one vector element into another), so that the earlier one can
            // then be zeroed or deleted.
            for (usize offset : {1, 2, 3})
            {
                for (usize i = 0; i + offset < run.Size(); ++i)
                {
                    usize j = i + offset;
                    if (run[i] == 0 || run[j] > ~0ull - run[i]) continue;

                    auto candidate = run;
                    candidate[j] += candidate[i];
                    candidate[i] = 0;
                    if (tryCandidate(candidate))
                    {
                        improved = true;
                        continue;
                    }

                    // Binary search for the largest amount that can move.
                    u64 low = 0, high = run[i];
                    while (low + 1 < high && attempts < MaxAttempts)
                    {
                        u64 middle   = low + (high - low) / 2;
                        u64 baseI    = run[i];
                        u64 baseJ    = run[j];
                        candidate    = run;
                        candidate[i] = baseI - middle;
                        candidate[j] = baseJ + middle;
                        if (tryCandidate(candidate))
                        {
                            improved = true;
                            // `run` changed: keep searching relative to it.
                            high -= middle;
                            low = 0;
                        }
                        else high = middle;
                    }
                }
            }
        }
        return run;
    }
} // namespace Prism::Test
