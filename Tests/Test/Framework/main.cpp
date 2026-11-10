/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Core/Error.hpp>
#include <Prism/Test/Test.hpp>

#include <signal.h>

using namespace Prism;

static int s_SetupRuns = 0;
TEST_SETUP { ++s_SetupRuns; }

TEST_CASE(setup_runs_before_each_case) { EXPECT(s_SetupRuns > 0); }

TEST_CASE(passing_expectations)
{
    EXPECT(true);
    EXPECT_EQ(2 + 2, 4);
    EXPECT_NE(1, 2);
    EXPECT_EQ_TRUTH(42, true);
    EXPECT_EQ_FORCE(String("a"), "a");
    EXPECT_APPROXIMATE(0.1 + 0.2, 0.3);
    EXPECT_APPROXIMATE_WITH_ERROR(1.0, 1.05, 0.1);
    VERIFY(1 < 2);
    EXPECT(!Test::CurrentTestCaseFailed());
}

TEST_CASE(equality_semantics)
{
    // C strings compare by content, integers by value across signedness.
    const char* a     = "text";
    char        b[]   = "text";
    EXPECT_EQ(a, b);
    EXPECT_EQ((Vector<int>{1, 2}.Size()), 2);
    EXPECT_NE(-1, 0xffffffffu);
    EXPECT_EQ(static_cast<u8>(200), 200);
}

TEST_CASE(printable_values)
{
    using Test::Details::ToPrintable;
    EXPECT_EQ(ToPrintable(42), "42"_s);
    EXPECT_EQ(ToPrintable(true), "true"_s);
    EXPECT_EQ(ToPrintable(1.5), "1.5"_s);
    EXPECT_EQ(ToPrintable(Vector<int>{1, 2, 3}), "[1, 2, 3]"_s);
    int array[] = {4, 5};
    EXPECT_EQ(ToPrintable(array), "[4, 5]"_s);
    struct Opaque
    {
    };
    EXPECT_EQ(ToPrintable(Opaque{}), "<unprintable>"_s);
}

TEST_CASE(try_or_fail_and_must)
{
    ErrorOr<int> value = 7;
    int          a     = TRY_OR_FAIL(value);
    EXPECT_EQ(a, 7);
    EXPECT_EQ(MUST(value), 7);
}

// Runs `body` in a child process with reporting disabled and reports whether
// it left the test failed (the parent's own test result stays untouched).
template <typename Fn>
static bool FailsTheTest(Fn body)
{
    return Test::Crash::Run("expectation failure is detected",
                            [&]
                            {
                                Test::DisableReporting();
                                body();
                                return Test::CurrentTestCaseFailed()
                                         ? Test::Crash::Failure::eDidNotCrash
                                         : Test::Crash::Failure::eUnexpectedError;
                            },
                            0);
}

TEST_CASE(failing_expectations_fail_the_test)
{
    EXPECT(FailsTheTest([] { EXPECT(false); }));
    EXPECT(FailsTheTest([] { EXPECT_EQ(1, 2); }));
    EXPECT(FailsTheTest([] { EXPECT_NE(3, 3); }));
    EXPECT(FailsTheTest([] { EXPECT_EQ_TRUTH(0, 1); }));
    EXPECT(FailsTheTest([] { EXPECT_APPROXIMATE(1.0, 2.0); }));
    EXPECT(FailsTheTest([] { FAIL("message"); }));
    EXPECT(FailsTheTest([] { VERIFY(false); }));
    EXPECT(FailsTheTest(
        []
        {
            ErrorOr<int> error = Error(ENOENT);
            [&] { (void)TRY_OR_FAIL(error); }();
        }));
}

TEST_CASE(crash_tests)
{
    EXPECT_CRASH("abort", [] { __builtin_abort(); });
    EXPECT_CRASH_WITH_SIGNAL("trap", SIGILL, [] { __builtin_trap(); });
    EXPECT_CRASH_WITH_SIGNAL("any signal", Test::Crash::AnySignal,
                             [] { raise(SIGTERM); });
    EXPECT_NO_CRASH("returns normally", [] {});
    EXPECT_NO_CRASH("returns DidNotCrash",
                    [] { return Test::Crash::Failure::eDidNotCrash; });

    // A function that doesn't crash (or dies the wrong way) makes
    // EXPECT_CRASH fail.
    Test::DisableReporting();
    bool noCrash     = Test::Crash::Run("doesn't crash", [] {});
    bool wrongSignal = Test::Crash::Run("wrong signal", [] { raise(SIGTERM); });
    Test::EnableReporting();
    EXPECT(!noCrash);
    EXPECT(!wrongSignal);
}

TEST_CASE(random_run_ordering)
{
    Test::RandomRun shorter(Vector<u64>{5, 5});
    Test::RandomRun longer(Vector<u64>{0, 0, 0});
    Test::RandomRun smaller(Vector<u64>{4, 9});
    EXPECT(shorter.IsShortlexSmallerThan(longer));
    EXPECT(smaller.IsShortlexSmallerThan(shorter));
    EXPECT(!shorter.IsShortlexSmallerThan(shorter));
    EXPECT_EQ(longer.WithoutChunk(1, 2).Size(), 1);
}

TEST_CASE(recorded_source_replays_and_clamps)
{
    Test::RandSource source = Test::RandSource::Recorded(
        Test::RandomRun(Vector<u64>{3, 100}));
    EXPECT_EQ(source.DrawValue(10), 3);
    EXPECT_EQ(source.DrawValue(10), 10);
}

RANDOMIZED_TEST_CASE(generators_respect_bounds)
{
    GEN(n, Gen::UnsignedInt(10, 20));
    EXPECT(n >= 10 && n <= 20);

    GEN(m, Gen::NumberI64(-5, 5));
    EXPECT(m >= -5 && m <= 5);

    GEN(v, Gen::Vector(2, 5, [] { return Gen::UnsignedInt(3); }));
    EXPECT(v.Size() >= 2 && v.Size() <= 5);
    for (auto x : v) EXPECT(x <= 3);

    GEN(p, Gen::Percentage());
    EXPECT(p >= 0.0 && p <= 1.0);

    GEN(c, Gen::OneOf('x', 'y', 'z'));
    EXPECT(c == 'x' || c == 'y' || c == 'z');

    GEN(f, Gen::Frequency(Gen::FrequencyOption{3, [] { return 1; }},
                          Gen::FrequencyOption{1, [] { return 2; }}));
    EXPECT(f == 1 || f == 2);

    GEN(s, Gen::String<String>(1, 4));
    EXPECT(s.Size() >= 1 && s.Size() <= 4);
}

RANDOMIZED_TEST_CASE(assume_rejects_some_inputs)
{
    GEN(n, Gen::UnsignedInt(10));
    ASSUME(n % 2 == 0);
    EXPECT_EQ(n % 2, 0u);
}

BENCHMARK_CASE(format_benchmark)
{
    for (int i = 0; i < 1000; ++i) (void)Format("{} {:#x}", i, i);
}
