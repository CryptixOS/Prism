/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism::Test
{
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
    TestResult CurrentTestResult();
    void       SetCurrentTestResult(TestResult result);
    /// Marks the current test as failed.
    void       CurrentTestCaseDidFail();
    /// Whether the current test has already failed.
    bool       CurrentTestCaseFailed();

    /// Expectation failures are only printed while reporting is enabled. It
    /// is disabled while a randomized test is searching for / shrinking a
    /// counterexample, and re-enabled for the final, reported run.
    bool       IsReportingEnabled();
    void       EnableReporting();
    void       DisableReporting();

    /// Number of runs of each RANDOMIZED_TEST_CASE (--randomized_runs).
    u64        RandomizedRuns();
} // namespace Prism::Test
