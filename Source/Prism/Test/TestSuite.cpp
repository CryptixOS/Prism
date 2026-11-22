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
import Prism.Test.Test;

namespace Prism::Test
{
    namespace
    {
        Vector<TestCase> s_Cases;
        TestFunction     s_Setup                = nullptr;
        TestResult       s_CurrentResult        = TestResult::eNotRun;
        bool             s_Reporting            = true;
        u64              s_RandomizedRuns       = 100;
        u64              s_BenchmarkRepetitions = 1;
        u64              s_Seed                 = 0;
        Random           s_Random;
        RandSource       s_RandSource;

    }; // namespace

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
        { return OutputIsTerminal() ? "\033[31;1mFAIL\033[0m" : "FAIL"; }
        static StringView PassTag()
        { return OutputIsTerminal() ? "\033[32;1mPASS\033[0m" : "PASS"; }

        String FormatDouble(double value)
        {
            char buffer[64];
            int  length = snprintf(buffer, sizeof(buffer), "%.17g", value);
            return String(buffer, length > 0 ? static_cast<usize>(length) : 0);
        }

        void ReportApproximateFailure(const char* lhsText, const char* rhsText,
                                      double lhs, double rhs, const char* file,
                                      u32 line)
        {
            if (Suite::IsReportingEnabled())
                Warn(
                    "{}: {}:{}: EXPECT_APPROXIMATE({}, {}) failed with "
                    "lhs={}, rhs={}, (lhs-rhs)={}",
                    FailTag(), file, line, lhsText, rhsText, FormatDouble(lhs),
                    FormatDouble(rhs), FormatDouble(lhs - rhs));
            CurrentTestCaseDidFail();
        }

        static u64 NowNanos()
        {
            timespec ts{};
            clock_gettime(CLOCK_MONOTONIC, &ts);
            return static_cast<u64>(ts.tv_sec) * 1'000'000'000ull
                 + static_cast<u64>(ts.tv_nsec);
        }
        static u64  ToMillis(u64 nanos) { return nanos / 1'000'000ull; }

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

    namespace Suite
    {
        void AddCase(const TestCase& testCase) { s_Cases.PushBack(testCase); }
        void SetSetup(TestFunction setup) { s_Setup = setup; }

        TestResult CurrentResult() { return s_CurrentResult; }
        void       SetCurrentResult(TestResult result)
        {
            // A failure sticks, unless the input turned out to be invalid.
            if (s_CurrentResult == TestResult::eFailed
                && result == TestResult::ePassed)
                return;
            s_CurrentResult = result;
        }

        bool        IsReportingEnabled() { return s_Reporting; }
        void        SetReportingEnabled(bool enabled) { s_Reporting = enabled; }

        u64         RandomizedRuns() { return s_RandomizedRuns; }
        RandSource& CurrentRandSource() { return s_RandSource; }

        static void PrintUsage(const char* program)
        {
            Warn("Usage: {} [options] [pattern]", program);
            Warn("  pattern                    only run matching cases (glob)");
            Warn("  --list                     list the available cases");
            Warn(
                "  --tests                    only run tests (or "
                "TESTS_ONLY=1)");
            Warn("  --bench                    only run benchmarks");
            Warn(
                "  --benchmark_repetitions N  times each benchmark runs "
                "(default 1)");
            Warn(
                "  --randomized_runs N        runs per RANDOMIZED_TEST_CASE "
                "(default 100)");
            Warn("  --seed N                   seed for randomized tests");
            Warn("  --help                     show this help");
        }

        int Main(int argc, char** argv)
        {
            bool        testsOnly      = getenv("TESTS_ONLY") != nullptr;
            bool        benchmarksOnly = false;
            bool        listCases      = false;
            bool        seedGiven      = false;
            StringView  pattern        = "*";
            const char* program        = argc > 0 ? argv[0] : "test";

            for (int i = 1; i < argc; ++i)
            {
                StringView arg          = argv[i];
                auto       numberOption = [&](StringView name, u64& out) -> int
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
                                                s_BenchmarkRepetitions))
                         != 0)
                    ;
                else if ((result
                          = numberOption("--randomized_runs", s_RandomizedRuns))
                         != 0)
                    ;
                else if ((result = numberOption("--seed", s_Seed)) != 0)
                    seedGiven = result > 0;
                else if (arg.Size() > 0 && arg[0] == '-')
                {
                    Warn("Unknown option '{}'", arg);
                    PrintUsage(program);
                    return 1;
                }
                else pattern = arg;

                if (result < 0)
                {
                    Warn("Invalid value for '{}'", arg);
                    return 1;
                }
            }

            if (listCases)
            {
                for (const auto& testCase : s_Cases)
                    Warn("    {}: {}",
                         testCase.IsBenchmark()    ? "benchmark"
                         : testCase.IsRandomized() ? "randomized"
                                                   : "test",
                         testCase.Name());
                return 0;
            }

            if (!seedGiven) s_Seed = NowNanos() ^ static_cast<u64>(getpid());
            s_Random.Seed(s_Seed);

            Vector<const TestCase*> selected;
            for (const auto& testCase : s_Cases)
            {
                if (testsOnly && testCase.IsBenchmark()) continue;
                if (benchmarksOnly && !testCase.IsBenchmark()) continue;
                if (pattern != "*" && !Matches(testCase.Name(), pattern))
                    continue;
                selected.PushBack(&testCase);
            }

            Warn("Running {} cases out of {}.", selected.Size(),
                 s_Cases.Size());

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

            Warn(
                "Finished {} tests and {} benchmarks in {}ms ({}ms tests, {}ms "
                "benchmarks, {}ms other).",
                stats.TestsRun, stats.BenchmarksRun, ToMillis(total),
                ToMillis(stats.TestNanos), ToMillis(stats.BenchmarkNanos),
                ToMillis(other));

            u64 failed = stats.TestsFailed + stats.BenchmarksFailed;
            if (failed == 0) Warn("All tests passed!");
            else
            {
                if (stats.TestsRun > 0)
                    Warn("Out of {} tests, {} passed and {} failed.",
                         stats.TestsRun, stats.TestsRun - stats.TestsFailed,
                         stats.TestsFailed);
                if (stats.BenchmarksRun > 0)
                    Warn("Out of {} benchmarks, {} passed and {} failed.",
                         stats.BenchmarksRun,
                         stats.BenchmarksRun - stats.BenchmarksFailed,
                         stats.BenchmarksFailed);
            }
            return failed > 0 ? 1 : 0;
        }

        bool RunCase(const TestCase& testCase, u64& nanos)
        {
            Warn("START  Running {} '{}'",
                 testCase.IsBenchmark()    ? "benchmark"
                 : testCase.IsRandomized() ? "randomized test"
                                           : "test",
                 testCase.Name());

            u64  start  = NowNanos();
            bool passed = testCase.IsBenchmark() ? RunBenchmark(testCase, nanos)
                        : testCase.IsRandomized() ? RunRandomized(testCase)
                                                  : RunTest(testCase);
            if (!testCase.IsBenchmark()) nanos = NowNanos() - start;

            if (passed)
                Warn("{}   {} ({}ms)", PassTag(), testCase.Name(),
                     ToMillis(nanos));
            else
                Warn("{}   {} ({}ms)", FailTag(), testCase.Name(),
                     ToMillis(nanos));
            return passed;
        }

        bool RunTest(const TestCase& testCase)
        {
            s_Reporting     = true;
            s_CurrentResult = TestResult::ePassed;
            if (s_Setup) s_Setup();
            testCase.Function()();
            return s_CurrentResult != TestResult::eFailed;
        }

        bool RunBenchmark(const TestCase& testCase, u64& nanos)
        {
            u64 repetitions
                = s_BenchmarkRepetitions > 0 ? s_BenchmarkRepetitions : 1;
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
                Warn("       {} repetitions: avg {}us, min {}us, max {}us",
                     repetitions, nanos / repetitions / 1000, best / 1000,
                     worst / 1000);
            return passed;
        }

        TestResult RunWithSource(const TestCase& testCase, RandSource source)
        {
            s_RandSource    = Move(source);
            s_CurrentResult = TestResult::ePassed;
            if (s_Setup) s_Setup();
            testCase.Function()();
            return s_CurrentResult;
        }

        bool RunRandomized(const TestCase& testCase)
        {
            // Give up when this many inputs in a row are rejected.
            constexpr u64 MaxRejectionsPerRun = 15;

            u64           passedRuns = 0, rejectedInARow = 0;
            while (passedRuns < s_RandomizedRuns)
            {
                s_Reporting = false;
                auto result
                    = RunWithSource(testCase, RandSource::Live(s_Random));
                if (result == TestResult::eRejected
                    || result == TestResult::eOverrun)
                {
                    if (++rejectedInARow >= MaxRejectionsPerRun)
                    {
                        s_Reporting = true;
                        Warn(
                            "{}: gave up after {} rejected inputs in a row "
                            "({} runs passed)",
                            FailTag(), rejectedInARow, passedRuns);
                        s_CurrentResult = TestResult::eHitLimit;
                        return false;
                    }
                    continue;
                }
                rejectedInARow = 0;

                if (result == TestResult::eFailed)
                {
                    RandomRun counterexample
                        = Shrink(testCase, s_RandSource.Run());

                    // Replay the minimal counterexample with reporting on, so
                    // GEN() prints the values and the expectations their
                    // errors.
                    s_Reporting = true;
                    Warn(
                        "Failed after {} successful runs; minimal "
                        "counterexample (seed {}):",
                        passedRuns, s_Seed);
                    RunWithSource(testCase,
                                  RandSource::Recorded(counterexample));
                    s_CurrentResult = TestResult::eFailed;
                    return false;
                }
                ++passedRuns;
            }
            s_Reporting = true;
            return true;
        }

        RandomRun Shrink(const TestCase& testCase, RandomRun run)
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
                    || !candidate.IsShortlexSmallerThan(run)
                    || !fails(candidate))
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
                                for (usize j = i;
                                     j > start
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
    }; // namespace Suite
} // namespace Prism::Test
