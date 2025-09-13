/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Queue.hpp>
using namespace Prism;

int main()
{
    using Q = Queue<int>;

    // --- Default construction ---
    {
        Q q;
        assert(q.Empty());
        assert(q.Size() == 0);
    }

    // --- Construction from container ---
    {
        Deque<int> d;
        d.PushBack(1);
        d.PushBack(2);
        Q q(d);
        assert(!q.Empty());
        assert(q.Size() == 2);
        assert(q.Front() == 1);
        assert(q.Back() == 2);
    }

    // --- Move construction from container ---
    {
        Deque<int> d;
        d.PushBack(42);
        Q q(std::move(d));
        assert(!q.Empty());
        assert(q.Front() == 42);
    }

    // --- Range construction ---
    {
        int arr[] = {1, 2, 3, 4};
        Q   q(arr, arr + 4);
        assert(q.Size() == 4);
        assert(q.Front() == 1);
        assert(q.Back() == 4);
    }

    // --- Push / Pop ---
    {
        Q q;
        q.Push(10);
        q.Push(20);
        q.Push(30);
        assert(q.Size() == 3);
        assert(q.Front() == 10);
        assert(q.Back() == 30);

        q.Pop();
        assert(q.Size() == 2);
        assert(q.Front() == 20);
    }

    // --- Push rvalue ---
    {
        Q   q;
        int x = 99;
        q.Push(std::move(x));
        assert(q.Size() == 1);
        assert(q.Front() == 99);
    }

    // --- Emplace ---
    {
        Queue<std::pair<int, int>> q;
        q.Emplace(1, 2);
        assert(q.Size() == 1);
        assert(q.Front().first == 1);
        assert(q.Front().second == 2);
    }

    // --- PopElement ---
    {
        Q q;
        q.Push(5);
        q.Push(6);
        int val = q.PopElement();
        assert(val == 5);
        assert(q.Size() == 1);
        assert(q.Front() == 6);
    }

    // --- Copy assignment ---
    {
        Q q1;
        q1.Push(1);
        q1.Push(2);
        Q q2;
        q2 = q1;
        assert(q2.Size() == 2);
        assert(q2.Front() == 1);
        assert(q2.Back() == 2);
    }

    // --- Move assignment ---
    {
        Q q1;
        q1.Push(42);
        Q q2;
        q2 = std::move(q1);
        assert(q2.Size() == 1);
        assert(q2.Front() == 42);
    }

    // --- Swap member ---
    {
        Q q1;
        q1.Push(1);
        Q q2;
        q2.Push(2);
        q1.Swap(q2);
        assert(q1.Front() == 2);
        assert(q2.Front() == 1);
    }

    // --- Swap free function ---
    {
        Q q1;
        q1.Push(10);
        Q q2;
        q2.Push(20);
        Swap(q1, q2);
        assert(q1.Front() == 20);
        assert(q2.Front() == 10);
    }

    return 0;
}
