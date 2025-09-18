/*
 * Created by v1tr10l7 on 17.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Utility/Atomic.hpp>

using namespace Prism;

enum class Color : u32
{
    eRed   = Bit(0),
    eGreen = Bit(1),
    eBlue  = Bit(2)
};

// --- Generic test function ---
// Generic test harness for Prism::Atomic<T>
template <typename T, template <typename> class Atomic = Atomic>
void TestAtomicClass()
{
    // convenience aliases
    using A = Atomic<T>;

    // --- Construction / Store / Load / operator= ---
    {
        A a(T(1));
        assert(a.Load() == T(1));
        a.Store(T(2), MemoryOrder::eSeqCst);
        assert(a.Load(MemoryOrder::eSeqCst) == T(2));

        // operator= returns previous value (per your Atomic::operator=
        // implementation)
        T prev = (a = T(3));
        assert(prev == T(2));
        assert(static_cast<T>(a) == T(3)); // operator T()

        // volatile variant
        volatile A va(T(4));
        assert(va.Load() == T(4));
        va.Store(T(5));
        assert(va.Load() == T(5));
    }

    // --- Exchange ---
    {
        A a(T(3));
        T old = a.Exchange(T(4), MemoryOrder::eSeqCst);
        assert(old == T(3));
        assert(a.Load() == T(4));

        volatile A va(T(6));
        T          oldv = va.Exchange(T(7));
        assert(oldv == T(6));
        assert(va.Load() == T(7));
    }

    // --- CompareExchange (strong) ---
    {
        A a(T(5));
        // success case
        {
            T    expected = T(5);
            bool ok       = a.CompareExchange(expected, T(8), /*weak*/ false,
                                              MemoryOrder::eSeqCst,
                                              MemoryOrder::eRelaxed);
            assert(ok);
            assert(a.Load() == T(8));
            // expected remains (usually set to original expected or
            // unspecified) - many wrappers leave it unchanged on success
        }

        // mismatch case - special-case for bool
        if constexpr (!IsSameV<T, bool>)
        {
            a.Store(T(5));
            T    expected = T(4);
            bool ok       = a.CompareExchange(expected, T(6), /*weak*/ false,
                                              MemoryOrder::eSeqCst,
                                              MemoryOrder::eRelaxed);
            // must fail because 5 != 4
            assert(!ok);
            assert(a.Load() == T(5));
            // expected must have been updated to current a.Load()
            assert(expected == T(5));
        }
        else
        {
            // bool-specific semantics: test true/false transitions
            a.Store(false);
            T    expected = T(false);
            bool ok
                = a.CompareExchange(expected, true, false, MemoryOrder::eSeqCst,
                                    MemoryOrder::eRelaxed);
            assert(ok);
            assert(a.Load() == true);

            expected = false;
            ok = a.CompareExchange(expected, false, false, MemoryOrder::eSeqCst,
                                   MemoryOrder::eRelaxed);
            // expected was false but a is true -> should fail and expected be
            // set to true
            assert(!ok);
            assert(a.Load() == true);
            assert(expected == true);
        }
    }

    // --- Arithmetic fetches (only for integral types and enums) ---
    if constexpr (IsIntegralV<T> || IsEnumV<T>)
    {
        A a(T(1));
        T res = a.FetchAdd(T(2), MemoryOrder::eSeqCst); // returns old
        // After fetch-add, new value should be old+2
        assert(res == T(1));
        assert(a.Load() == T(3));

        res = a.FetchSub(T(1), MemoryOrder::eSeqCst);
        assert(res == T(3));
        assert(a.Load() == T(2));
    }

    // --- Bitwise fetches (only for integral types, exclude bool) ---
    if constexpr (IsIntegralV<T> && !IsSameV<T, bool>)
    {
        A a(T(1));
        T old = a.FetchOr(T(4), MemoryOrder::eSeqCst);
        assert(old == T(1));
        assert(a.Load() == T(1 | 4));

        a.Store(T(1));
        old = a.FetchAnd(T(1), MemoryOrder::eSeqCst);
        assert(old == T(1));
        assert(a.Load() == T(1 & 1));

        a.Store(T(1));
        old = a.FetchXor(T(2), MemoryOrder::eSeqCst);
        assert(old == T(1));
        assert(a.Load() == T(1 ^ 2));
    }

    // --- FetchAdd / FetchSub operator++ / operator-- / compound assignment ---
    if constexpr (!IsSameV<T, bool>)
    {
        A a(T(0));

        // prefix ++
        T pref = ++a;
        assert(a.Load() == T(1));
        assert(pref == a.Load());

        // postfix ++ returns previous value
        T post = a++;
        assert(post == T(1));
        assert(a.Load() == T(2));

        // prefix --
        T predec = --a;
        assert(predec == a.Load());
        assert(a.Load() == T(1));

        // postfix --
        T postdec = a--;
        assert(postdec == T(1));
        assert(a.Load() == T(0));

        // compound assignment
        T addres = (a += T(5));
        assert(a.Load() == T(5));
        assert(addres == a.Load());

        T subres = (a -= T(2));
        assert(a.Load() == T(3));
        assert(subres == a.Load());
    }

    // --- Bitwise compound assignments (integral only, exclude bool) ---
    if constexpr (IsIntegralV<T> && !IsSameV<T, bool>)
    {
        A a(T(6));
        T andres = (a &= T(3));
        assert(andres == a.Load());
        A b(T(1));
        T orres = (b |= T(4));
        assert(orres == b.Load());
        A c(T(2));
        T xorres = (c ^= T(1));
        assert(xorres == c.Load());
    }

    // --- IsLockFree / IsAlwaysLockFree ---
    {
        A    a(T(0));
        bool lockfree_instance = a.IsLockFree();
        bool lockfree_static   = A::IsAlwaysLockFree();
        // We can't assert they are true on all targets; only ensure the calls
        // compile and return a bool
        (void)lockfree_instance;
        (void)lockfree_static;
    }

    // --- volatile overloads basic smoke tests ---
    {
        volatile A va(T(10));
        assert(va.Load() == T(10));
        va.Store(T(11));
        assert(va.Load() == T(11));
        T old = va.Exchange(T(12));
        assert(old == T(11));
        (void)old;
    }

    // --- boolean-specific atomic test_and_set / clear style behavior ---
    if constexpr (IsSameV<T, bool>)
    {
        A    a(false);
        // test as a flag using CompareExchange (since Atomic class doesn't
        // expose test_and_set directly)
        T    expected = false;
        bool ok = a.CompareExchange(expected, true, false, MemoryOrder::eSeqCst,
                                    MemoryOrder::eRelaxed);
        assert(ok);
        assert(a.Load() == true);
        // Clear via Store
        a.Store(false);
        assert(a.Load() == false);
    }

    // final smoke: load/store with different memory orders compile
    {
        A a(T(0));
        a.Store(T(1), MemoryOrder::eRelaxed);
        (void)a.Load(MemoryOrder::eAcquire);
    }
}

// Runner: instantiate tests for a set of types
int main()
{
    TestAtomicClass<i8>();
    TestAtomicClass<i16>();
    TestAtomicClass<i32>();
    TestAtomicClass<i64>();
    TestAtomicClass<u8>();
    TestAtomicClass<u16>();
    TestAtomicClass<u32>();
    TestAtomicClass<u64>();
    TestAtomicClass<bool>();
    TestAtomicClass<Color>();

    return 0;
}
