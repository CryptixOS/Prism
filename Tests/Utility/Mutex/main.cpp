/*
 * Created by v1tr10l7 on 09.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Test.Test;
import Prism.Utility.LockingPolicy;
import Prism.Utility.Math;
import Prism.Utility.Mutex;

using namespace Prism;

static_assert(LockingPolicy<Spinlock>);
static_assert(LockingPolicy<Mutex>);

template <typename Lock>
static void CheckLock()
{
    Lock lock;
    EXPECT(!lock.IsLocked());

    lock.Lock();
    EXPECT(lock.IsLocked());
    EXPECT(!lock.TryLock());
    lock.Unlock();
    EXPECT(!lock.IsLocked());

    EXPECT(lock.TryLock());
    lock.Unlock();

    {
        ScopedLock guard(lock);
        EXPECT(lock.IsLocked());
    }
    EXPECT(!lock.IsLocked());
}

TEST_CASE(spinlock) { CheckLock<Spinlock>(); }
TEST_CASE(mutex) { CheckLock<Mutex>(); }

TEST_CASE(fabs_values)
{
    EXPECT_EQ(Math::FAbs(-2.5), 2.5);
    EXPECT_EQ(Math::FAbs(2.5f), 2.5f);
    EXPECT(!__builtin_signbit(Math::FAbs(-0.0)));
    static_assert(Math::FAbs(-1.0) == 1.0);
    EXPECT_EQ(Math::Absolute(-3.0), 3.0);
}

TEST_CASE(absolute_integral)
{
    EXPECT_EQ(Math::Absolute(-5), 5u);
    EXPECT_EQ(Math::Absolute(5), 5u);
    // The minimum value has no positive counterpart in its own type.
    EXPECT_EQ(Math::Absolute(static_cast<i32>(-2147483647 - 1)),
              2147483648u);
    static_assert(Math::Absolute(static_cast<i8>(-128)) == 128);
}
