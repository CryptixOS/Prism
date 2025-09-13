/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Stack.hpp>
using namespace Prism;

int main()
{
    using S = Stack<int>;

    // --- Default construction ---
    {
        S s;
        assert(s.Empty());
        assert(s.Size() == 0);
    }

    // --- Construction from container ---
    {
        Deque<int> d;
        d.PushBack(1);
        d.PushBack(2);
        S s(d);
        assert(!s.Empty());
        assert(s.Size() == 2);
        assert(s.Front() == 1);
        assert(s.Back() == 2);
    }

    // --- Move construction from container ---
    {
        Deque<int> d;
        d.PushBack(42);
        S s(std::move(d));
        assert(!s.Empty());
        assert(s.Front() == 42);
    }

    // --- Range construction ---
    {
        int arr[] = {1, 2, 3, 4};
        S   s(arr, arr + 4);
        assert(s.Size() == 4);
        assert(s.Front() == 1);
        assert(s.Back() == 4);
    }

    // --- Push / Pop ---
    {
        S s;
        s.Push(10);
        s.Push(20);
        s.Push(30);
        assert(s.Size() == 3);
        assert(s.Back() == 30);

        s.Pop();
        assert(s.Size() == 2);
        assert(s.Back() == 20);
    }

    // --- Push rvalue ---
    {
        S   s;
        int x = 99;
        s.Push(std::move(x));
        assert(s.Size() == 1);
        assert(s.Back() == 99);
    }

    // --- Emplace ---
    {
        Stack<std::pair<int, int>> s;
        s.Emplace(1, 2);
        assert(s.Size() == 1);
        assert(s.Back().first == 1);
        assert(s.Back().second == 2);
    }

    // --- PopElement ---
    {
        S s;
        s.Push(5);
        s.Push(6);
        int val = s.PopElement();
        assert(val == 6);
        assert(s.Size() == 1);
        assert(s.Back() == 5);
    }

    // --- Copy assignment ---
    {
        S s1;
        s1.Push(1);
        s1.Push(2);
        S s2;
        s2 = s1;
        assert(s2.Size() == 2);
        assert(s2.Front() == 1);
        assert(s2.Back() == 2);
    }

    // --- Move assignment ---
    {
        S s1;
        s1.Push(42);
        S s2;
        s2 = std::move(s1);
        assert(s2.Size() == 1);
        assert(s2.Back() == 42);
    }

    // --- Swap member ---
    {
        S s1;
        s1.Push(1);
        S s2;
        s2.Push(2);
        s1.Swap(s2);
        assert(s1.Back() == 2);
        assert(s2.Back() == 1);
    }

    // --- Swap free function ---
    {
        S s1;
        s1.Push(10);
        S s2;
        s2.Push(20);
        Swap(s1, s2);
        assert(s1.Back() == 20);
        assert(s2.Back() == 10);
    }

    return 0;
}
