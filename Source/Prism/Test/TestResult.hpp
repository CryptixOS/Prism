/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism::Test
{
    class RandSource;
    enum class TestResult : u8;
    namespace Suite
    {
        TestResult  CurrentResult();
        void        SetCurrentResult(TestResult result);

        bool        IsReportingEnabled();
        void        SetReportingEnabled(bool enabled);

        u64         RandomizedRuns();
        RandSource& CurrentRandSource();
    }; // namespace Suite

    enum class TestResult : u8
    {
        /// The test hasn't run yet.
        eNotRun,
        /// Every expectation held.
        ePassed,
        /// At least one expectation failed.
        eFailed,
        /// The generated input was rejected by ASSUME()/REJECT(); the run
        /// does not count (randomized tests only).
        eRejected,
        /// A replayed RandomRun ran out of values (randomized tests only,
        /// happens while shrinking).
        eOverrun,
        /// Too many generated inputs were rejected (randomized tests only).
        eHitLimit,
    };

    constexpr const char* ToString(TestResult result)
    {
        switch (result)
        {
            case TestResult::eNotRun: return "NotRun";
            case TestResult::ePassed: return "Passed";
            case TestResult::eFailed: return "Failed";
            case TestResult::eRejected: return "Rejected";
            case TestResult::eOverrun: return "Overrun";
            case TestResult::eHitLimit: return "HitLimit";
        }
        return "Unknown";
    }

    // State of the test that is currently running (see TestSuite).
    inline constexpr TestResult CurrentTestResult()
    { return Suite::CurrentResult(); }
    inline constexpr void SetCurrentTestResult(TestResult result)
    { return Suite::SetCurrentResult(result); }
    /// Marks the current test as failed.
    inline constexpr void CurrentTestCaseDidFail()
    { Suite::SetCurrentResult(TestResult::eFailed); }
    /// Whether the current test has already failed.
    inline constexpr bool CurrentTestCaseFailed()
    { return Suite::CurrentResult() == TestResult::eFailed; }

    /// Expectation failures are only printed while reporting is enabled. It
    /// is disabled while a randomized test is searching for / shrinking a
    /// counterexample, and re-enabled for the final, reported run.
    inline constexpr bool IsReportingEnabled()
    { return Suite::IsReportingEnabled(); }
    inline constexpr void EnableReporting()
    { Suite::SetReportingEnabled(true); }
    inline constexpr void DisableReporting()
    { Suite::SetReportingEnabled(false); }

    /// Number of runs of each RANDOMIZED_TEST_CASE (--randomized_runs).
    inline constexpr u64 RandomizedRuns() { return Suite::RandomizedRuns(); }
} // namespace Prism::Test
