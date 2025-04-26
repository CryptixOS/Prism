/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/IntrusiveList.hpp>

namespace Prism
{
    template <typename T>
    void IntrusiveListHook<T>::Unlink(T* self)
    {
        if (!Owner) return;

        if (Previous)
        {
            auto& previousHook = Hook(Previous);
            previousHook.Next  = Next;
        }
        else Owner->m_Head = Next;

        if (Next)
        {

            auto& nextHook    = Hook(Next);
            nextHook.Previous = Previous;
        }
        else Owner->m_Tail = Previous;

        Previous = nullptr;
        Next     = nullptr;
        Owner    = nullptr;
    }

    template <typename T, typename HookType>
    constexpr void IntrusiveList<T, HookType>::PushFront(T* node)
    {
        auto& hook = HookType::Hook(node);
        assert(!hook.IsLinked());

        hook.Previous = nullptr;
        hook.Next     = m_Head;
        hook.Owner    = this;

        if (m_Head)
        {
            auto& headHook    = HookType::Hook(m_Head);
            headHook.Previous = node;
        }
        else m_Tail = node;

        m_Head = node;
    }

    template <typename T, typename HookType>
    constexpr void IntrusiveList<T, HookType>::PushBack(T* node)
    {
        auto& hook = HookType::Hook(node);
        assert(!hook.IsLinked());

        hook.Next     = nullptr;
        hook.Previous = m_Tail;
        hook.Owner    = this;

        if (m_Tail)
        {
            auto& tailHook = HookType::Hook(m_Tail);
            tailHook.Next  = node;
        }
        else m_Head = node;

        m_Tail = node;
    }

    template <typename T, typename HookType>
    constexpr void IntrusiveList<T, HookType>::PopFront()
    {
        if (!m_Head) return;
        Erase(m_Head);
    }
    template <typename T, typename HookType>
    constexpr void IntrusiveList<T, HookType>::PopBack()
    {
        if (!m_Tail) return;
        Erase(m_Tail);
    }

    template <typename T, typename HookType>
    constexpr void IntrusiveList<T, HookType>::Erase(T* node)
    {
        auto& hook = HookType::Hook(node);
        hook.Unlink(node);
    }
    template <typename T, typename HookType>
    constexpr void IntrusiveList<T, HookType>::Clear()
    {
        while (m_Head) Erase(m_Head);
    }

    template <typename T, typename HookType>
    constexpr IntrusiveList<T, HookType>::Iterator&
    IntrusiveList<T, HookType>::Iterator::operator++()
    {
        if (Current) Current = HookType::Hook(Current).Next;

        return *this;
    }
    template <typename T, typename HookType>
    constexpr IntrusiveList<T, HookType>::Iterator
    IntrusiveList<T, HookType>::Iterator::operator++(int)
    {
        Iterator temp = *this;
        ++(*this);

        return temp;
    }
}; // namespace Prism
