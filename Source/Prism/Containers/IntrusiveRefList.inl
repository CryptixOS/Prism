/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/IntrusiveRefList.hpp>

namespace Prism
{
    // IntrusiveRefListHook
    template <typename T>
    void IntrusiveRefListHook<T>::Unlink(Ref<T> self)
    {
        if (!Owner) return;

        if (self == Owner->m_Head) Owner->m_Head = Previous;
        else if (self == Owner->m_Tail) Owner->m_Tail = Next;

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

    // IntrusiveRefList::Iterator
    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator&
    IntrusiveRefList<T, HookType>::Iterator::operator++()
    {
        if (Current) Current = HookType::Hook(Current).Next;

        return *this;
    }
    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::Iterator::operator++(int)
    {
        Iterator temp = *this;
        ++(*this);

        return temp;
    }

    // IntrusiveRefList
    template <typename T, typename HookType>
    inline constexpr IntrusiveRefList<T, HookType>::IntrusiveRefList(
        const IntrusiveRefList& other)
    {
        *this = other;
    }
    template <typename T, typename HookType>
    inline constexpr IntrusiveRefList<T, HookType>::IntrusiveRefList(
        IntrusiveRefList&& other)
    {
        *this = Move(other);
    }

    template <typename T, typename HookType>
    inline constexpr IntrusiveRefList<T, HookType>::~IntrusiveRefList()
    {
        Clear();
    }

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>&
    IntrusiveRefList<T, HookType>::operator=(const IntrusiveRefList& other)
    {
        if (m_Head) Clear();
        auto otherCurrent = other.m_Head;
        auto current = m_Head = new T(*other.m_Head);
        current->Hook.Owner   = this;

        while (otherCurrent)
        {
            otherCurrent        = otherCurrent->Hook.Next;
            auto next           = new T(*otherCurrent);

            next->Hook.Previous = current;
            current->Hook.Owner = this;
            current->Hook.Next  = next;
            current             = next;
        }

        return *this;
    }
    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>&
    IntrusiveRefList<T, HookType>::operator=(IntrusiveRefList&& other)
    {
        if (m_Head) Clear();

        m_Head       = Move(other.m_Head);
        m_Tail       = Move(other.m_Tail);
        // m_Size       = Move(other.m_Size);

        other.m_Head = nullptr;
        other.m_Tail = nullptr;
        other.m_Size = 0;

        // Change ownership
        auto next    = Head();
        Ref<T>   current = nullptr;
        while (next)
        {
            current = next;
            if (!current) break;

            next                = current->Hook.Next;
            current->Hook.Owner = this;
        }

        return *this;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline constexpr bool
    IntrusiveRefList<T, HookType>::Empty() const
    {
        return !m_Head;
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline constexpr usize
    IntrusiveRefList<T, HookType>::Size() const
    {
        return m_Size;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline constexpr Ref<T> IntrusiveRefList<T, HookType>::Head()
    {
        return m_Head;
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline constexpr const Ref<T>
    IntrusiveRefList<T, HookType>::Head() const
    {
        return m_Head;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline constexpr Ref<T> IntrusiveRefList<T, HookType>::Tail()
    {
        return m_Tail;
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline constexpr const Ref<T>
    IntrusiveRefList<T, HookType>::Tail() const
    {
        return m_Tail;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::begin()
    {
        return Iterator(m_Head);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::begin() const
    {
        return Iterator(m_Head);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::cbegin() const PM_NOEXCEPT
    {
        return Iterator(m_Head);
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::end()
    {
        return Iterator(nullptr);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::end() const
    {
        return Iterator(nullptr);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::cend() const PM_NOEXCEPT
    {
        return Iterator(nullptr);
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rbegin()
    {
        return Iterator(nullptr);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rbegin() const
    {
        return Iterator(nullptr);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::crbegin() const PM_NOEXCEPT
    {
        return Iterator(nullptr);
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rend()
    {
        return Iterator(m_Head);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rend() const
    {
        return Iterator(m_Head);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE inline const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::crend() const PM_NOEXCEPT
    {
        return Iterator(m_Head);
    }

    template <typename T, typename HookType>
    constexpr void IntrusiveRefList<T, HookType>::Clear()
    {
        while (m_Head) Erase(m_Head);

        m_Head = nullptr;
        m_Tail = nullptr;
        m_Size = 0;
    }

    template <typename T, typename HookType>
    template <typename... Args>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::EmplaceFront(Args&&... args)
    {
        PushFront(T(Forward<Args>(args)...));

        return Head();
    }
    template <typename T, typename HookType>
    template <typename... Args>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::EmplaceBack(Args&&... args)
    {
        PushBack(T(Forward<Args>(args)...));

        return Tail();
    }

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::PushFront(Ref<T> node)
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
        ++m_Size;
        return Iterator(node);
    }

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::PushBack(Ref<T> node)
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
        ++m_Size;
        return Iterator(node);
    }

    template <typename T, typename HookType>
    constexpr void IntrusiveRefList<T, HookType>::PopFront()
    {
        if (!m_Head) return;
        Erase(m_Head);
    }
    template <typename T, typename HookType>
    constexpr void IntrusiveRefList<T, HookType>::PopBack()
    {
        if (!m_Tail) return;
        Erase(m_Tail);
    }

    template <typename T, typename HookType>
    constexpr Ref<T> IntrusiveRefList<T, HookType>::PopFrontElement()
    {
        auto head = Head();
        PopFront();

        return head;
    }
    template <typename T, typename HookType>
    constexpr Ref<T> IntrusiveRefList<T, HookType>::PopBackElement()
    {
        auto tail = Tail();
        PopBack();

        return tail;
    }

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::Erase(Iterator it)
    {
        auto& hook = HookType::Hook(it.Current);
        Ref<T>    next = HookType::Hook(it.Current).Next;

        hook.Unlink(it.Current);
        --m_Size;
        return next ? Iterator(next) : end();
    }
    template <typename T, typename HookType>
    constexpr void IntrusiveRefList<T, HookType>::Erase(Ref<T> node)
    {
        auto& hook = HookType::Hook(node);
        hook.Unlink(node);

        --m_Size;
    }
}; // namespace Prism
