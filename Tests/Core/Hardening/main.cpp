/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Containers.Array;
import Prism.Containers.Deque;
import Prism.Containers.Span;
import Prism.Containers.Vector;
import Prism.String.String;
import Prism.String.StringView;
import Prism.Test.Test;
import Prism.Utility.Expected;
import Prism.Utility.Optional;

using namespace Prism;

// Precondition violations must stop the program rather than read or write
// out of bounds: through PrismCheck, a trap with PRISM_HARDENED and an
// assert() in debug builds. Release builds without PRISM_HARDENED check
// nothing, so there is nothing to test there.
#if PRISM_HARDENED != 0 || !defined(NDEBUG)
    #define PRISM_CHECKS_ENABLED 1
#else
    #define PRISM_CHECKS_ENABLED 0
#endif

// Keeps the compiler from proving the index out of bounds at compile time.
template <typename T>
static T Opaque(T value)
{
    __asm__ volatile("" : "+r"(value));
    return value;
}

TEST_CASE(containers_check_bounds)
{
#if PRISM_CHECKS_ENABLED
    EXPECT_CHECK_FAILURE("Vector[] past the end", ([] {
        Vector<int> v = {1, 2, 3};
        (void)v[Opaque(3uz)];
    }));
    EXPECT_CHECK_FAILURE("Vector::Front() of empty", ([] {
        Vector<int> v;
        (void)v.Front();
    }));
    EXPECT_CHECK_FAILURE("Vector::Erase(end())", ([] {
        Vector<int> v = {1};
        v.Erase(v.end());
    }));
    EXPECT_CHECK_FAILURE("Array[] past the end", ([] {
        Array<int, 4> a{};
        (void)a[Opaque(4uz)];
    }));
    EXPECT_CHECK_FAILURE("Span[] past the end", ([] {
        int       data[2] = {1, 2};
        Span<int> s(data, 2);
        (void)s[Opaque(2uz)];
    }));
    EXPECT_CHECK_FAILURE("Deque::Front() of empty", ([] {
        Deque<int> d;
        (void)d.Front();
    }));
#endif
}

TEST_CASE(strings_check_bounds)
{
#if PRISM_CHECKS_ENABLED
    EXPECT_CHECK_FAILURE("String[] past the end", ([] {
        String s = "abc";
        (void)s[Opaque(3uz)];
    }));
    EXPECT_CHECK_FAILURE("StringView[] past the end", ([] {
        StringView s = "abc";
        (void)s[Opaque(10uz)];
    }));
    EXPECT_CHECK_FAILURE("StringView::Substr() past the end", ([] {
        StringView s = "abc";
        (void)s.Substr(Opaque(4uz));
    }));
#endif
}

TEST_CASE(sum_types_check_access)
{
#if PRISM_CHECKS_ENABLED
    EXPECT_CHECK_FAILURE("Optional::Value() of empty", ([] {
        Optional<int> o;
        (void)o.Value();
    }));
    EXPECT_CHECK_FAILURE("Expected::Value() of an error", ([] {
        Expected<int, int> e = Unexpected<int>(5);
        (void)e.Value();
    }));
#endif
}

TEST_CASE(valid_accesses_still_work)
{
    Vector<int> v = {1, 2, 3};
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v.Front(), 1);
    EXPECT_EQ(v.Back(), 3);
    v.PopBack();
    EXPECT_EQ(v.Size(), 2u);

    StringView s = "abc";
    EXPECT_EQ(s[2], 'c');
    EXPECT(s.Substr(3).Empty());
    EXPECT(s.Substr(1, 100) == "bc");
}
