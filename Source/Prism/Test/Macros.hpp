/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

// Macros only: the code lives in module Prism.Test.Macros, which
// files using these macros import too.

/// The current test fails unless `x` is true.
#define EXPECT(...)                                                            \
    do                                                                         \
    {                                                                          \
        if (!static_cast<bool>(__VA_ARGS__))                                   \
            ::Prism::Test::Details::ReportFailure(                             \
                "EXPECT(" #__VA_ARGS__ ") failed", __FILE__, __LINE__);        \
    } while (false)

/// The current test fails unless `a == b`; prints both values.
#define EXPECT_EQ(a, b)                                                        \
    do                                                                         \
    {                                                                          \
        auto&& prismExpectLhs = (a);                                           \
        auto&& prismExpectRhs = (b);                                           \
        if (!::Prism::Test::Details::Equals(prismExpectLhs, prismExpectRhs))   \
            ::Prism::Test::Details::ReportComparisonFailure(                   \
                "EXPECT_EQ", #a, #b, prismExpectLhs, prismExpectRhs, __FILE__, \
                __LINE__);                                                     \
    } while (false)

/// The current test fails unless `a` and `b` have the same truthiness.
#define EXPECT_EQ_TRUTH(a, b)                                                  \
    do                                                                         \
    {                                                                          \
        auto&& prismExpectLhs = (a);                                           \
        auto&& prismExpectRhs = (b);                                           \
        if (static_cast<bool>(prismExpectLhs)                                  \
            != static_cast<bool>(prismExpectRhs))                              \
            ::Prism::Test::Details::ReportTruthFailure(                        \
                #a, #b, prismExpectLhs, prismExpectRhs, __FILE__, __LINE__);   \
    } while (false)

/// Like EXPECT_EQ, but refuses to compile if the values aren't printable,
/// for when EXPECT_EQ prints "<unprintable>".
#define EXPECT_EQ_FORCE(a, b)                                                  \
    do                                                                         \
    {                                                                          \
        auto&& prismExpectLhs = (a);                                           \
        auto&& prismExpectRhs = (b);                                           \
        static_assert(::Prism::Formattable<decltype(prismExpectLhs)>           \
                          && ::Prism::Formattable<decltype(prismExpectRhs)>,   \
                      "EXPECT_EQ_FORCE requires formattable values");          \
        if (!::Prism::Test::Details::Equals(prismExpectLhs, prismExpectRhs))   \
            ::Prism::Test::Details::ReportComparisonFailure(                   \
                "EXPECT_EQ_FORCE", #a, #b, prismExpectLhs, prismExpectRhs,     \
                __FILE__, __LINE__);                                           \
    } while (false)

/// The current test fails if `a == b`.
#define EXPECT_NE(a, b)                                                        \
    do                                                                         \
    {                                                                          \
        auto&& prismExpectLhs = (a);                                           \
        auto&& prismExpectRhs = (b);                                           \
        if (::Prism::Test::Details::Equals(prismExpectLhs, prismExpectRhs))    \
            ::Prism::Test::Details::ReportComparisonFailure(                   \
                "EXPECT_NE", #a, #b, prismExpectLhs, prismExpectRhs, __FILE__, \
                __LINE__);                                                     \
    } while (false)

/// The current test fails unless |a - b| <= err.
#define EXPECT_APPROXIMATE_WITH_ERROR(a, b, err)                               \
    do                                                                         \
    {                                                                          \
        double prismExpectLhs = static_cast<double>(a);                        \
        double prismExpectRhs = static_cast<double>(b);                        \
        double prismExpectErr = static_cast<double>(err);                      \
        if (!(__builtin_fabs(prismExpectLhs - prismExpectRhs)                  \
              <= prismExpectErr))                                              \
            ::Prism::Test::Details::ReportApproximateFailure(                  \
                #a, #b, prismExpectLhs, prismExpectRhs, __FILE__, __LINE__);   \
    } while (false)

#define EXPECT_APPROXIMATE(a, b) EXPECT_APPROXIMATE_WITH_ERROR(a, b, 0.0000005)

/// Fails the current test with a message (anything printable).
#define FAIL(message)                                                          \
    ::Prism::Test::Details::ReportFailure(                                     \
        ::Prism::Test::Details::ToPrintable(message), __FILE__, __LINE__)

/// Rejects the generated input of a randomized test (the run doesn't count).
#define REJECT(message)                                                        \
    do                                                                         \
    {                                                                          \
        if (::Prism::Test::IsReportingEnabled())                               \
            ::Prism::Test::Warn("REJECTED test case: {}",                      \
                                ::Prism::Test::Details::ToPrintable(message)); \
        ::Prism::Test::SetCurrentTestResult(                                   \
            ::Prism::Test::TestResult::eRejected);                             \
    } while (false)

/// Rejects the generated input of a randomized test unless `x` holds, and
/// returns from the test.
#define ASSUME(x)                                                              \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            if (::Prism::Test::IsReportingEnabled())                           \
                ::Prism::Test::Warn("{}:{}: Assumption failed: {}", __FILE__,  \
                                    __LINE__, #x);                             \
            ::Prism::Test::SetCurrentTestResult(                               \
                ::Prism::Test::TestResult::eRejected);                         \
            return;                                                            \
        }                                                                      \
    } while (false)

/// Generates a value for a randomized test and prints it when reporting a
/// counterexample: `GEN(n, Gen::UnsignedInt(100));`.
#define GEN(identifier, value)                                                 \
    auto identifier = (value);                                                 \
    if (::Prism::Test::CurrentTestResult()                                     \
        == ::Prism::Test::TestResult::eOverrun)                                \
        return;                                                                \
    ::Prism::Test::Details::ReportGenerated(#identifier, identifier)

/// Unwraps an Expected; on error fails the test and returns from it.
#define TRY_OR_FAIL(expression)                                                \
    ({                                                                         \
        auto&& prismTryResult = (expression);                                  \
        if (!prismTryResult) [[unlikely]]                                      \
        {                                                                      \
            FAIL(::Prism::Test::Details::DescribeError(prismTryResult));       \
            return;                                                            \
        }                                                                      \
        ::Prism::Move(prismTryResult.Value());                                 \
    })

#ifndef MUST
  /// Unwraps an Expected/Optional; on error reports and aborts.
    #define MUST(expression)                                                   \
        ({                                                                     \
            auto&& prismMustResult = (expression);                             \
            if (!prismMustResult) [[unlikely]]                                 \
            {                                                                  \
                ::Prism::Test::Warn(                                           \
                    "{}: {}:{}: MUST({}) failed: {}",                          \
                    ::Prism::Test::Details::FailTag(), __FILE__, __LINE__,     \
                    #expression,                                               \
                    ::Prism::Test::Details::DescribeError(prismMustResult));   \
                __builtin_trap();                                              \
            }                                                                  \
            ::Prism::Move(prismMustResult.Value());                            \
        })
#endif

// In tests, VERIFY() reports instead of aborting; VERIFY_NOT_REACHED() and
// TODO() still abort, as the code after them can't run meaningfully.
#undef VERIFY
#define VERIFY(...)                                                            \
    do                                                                         \
    {                                                                          \
        if (!(__VA_ARGS__))                                                    \
            ::Prism::Test::Details::ReportFailure(                             \
                "VERIFY(" #__VA_ARGS__ ") failed", __FILE__, __LINE__);        \
    } while (false)
#undef VERIFY_NOT_REACHED
#define VERIFY_NOT_REACHED()                                                   \
    do                                                                         \
    {                                                                          \
        ::Prism::Test::Warn("{}: {}:{}: VERIFY_NOT_REACHED() called",          \
                            ::Prism::Test::Details::FailTag(), __FILE__,       \
                            __LINE__);                                         \
        __builtin_abort();                                                     \
    } while (false)
#undef TODO
#define TODO()                                                                 \
    do                                                                         \
    {                                                                          \
        ::Prism::Test::Warn("{}: {}:{}: TODO() called",                        \
                            ::Prism::Test::Details::FailTag(), __FILE__,       \
                            __LINE__);                                         \
        __builtin_abort();                                                     \
    } while (false)

/**
 * Runs `test_func` in a child process and expects it to crash with SIGABRT
 * (EXPECT_CRASH), a given signal (EXPECT_CRASH_WITH_SIGNAL, or
 * Test::Crash::AnySignal) or to exit normally (EXPECT_NO_CRASH). The
 * function may return void or Test::Crash::Failure.
 */
#define EXPECT_CRASH(test_message, test_func)                                  \
    do                                                                         \
    {                                                                          \
        if (!::Prism::Test::Crash::Run(test_message, test_func))               \
            ::Prism::Test::CurrentTestCaseDidFail();                           \
    } while (false)

#define EXPECT_CRASH_WITH_SIGNAL(test_message, signal, test_func)              \
    do                                                                         \
    {                                                                          \
        if (!::Prism::Test::Crash::Run(test_message, test_func, signal))       \
            ::Prism::Test::CurrentTestCaseDidFail();                           \
    } while (false)

#define EXPECT_NO_CRASH(test_message, test_func)                               \
    do                                                                         \
    {                                                                          \
        if (!::Prism::Test::Crash::Run(test_message, test_func, 0))            \
            ::Prism::Test::CurrentTestCaseDidFail();                           \
    } while (false)
