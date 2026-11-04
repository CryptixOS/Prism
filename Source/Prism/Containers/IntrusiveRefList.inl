/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/IntrusiveRefList.hpp>

PRISM_EXPORT namespace Prism
{
    // IntrusiveRefListHook
    template <typename T, typename Pointer>
    void IntrusiveRefListHook<T, Pointer>::Unlink(PointerType self)
    {
        if (!Owner) return;

        if (Previous) Hook(Previous).Next = Next;
        else Owner->m_Head = Next;

        if (Next) Hook(Next).Previous = Previous;
        else Owner->m_Tail = Previous;

        --Owner->m_Size;

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

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator&
    IntrusiveRefList<T, HookType>::Iterator::operator--()
    {
        if (Current) Current = HookType::Hook(Current).Previous;
        else if (List) Current = List->m_Tail;

        return *this;
    }
    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::Iterator::operator--(int)
    {
        Iterator temp = *this;
        --(*this);

        return temp;
    }

    // IntrusiveRefList
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
    IntrusiveRefList<T, HookType>::operator=(IntrusiveRefList&& other)
    {
        if (this == &other) return *this;
        Clear();

        m_Head       = Move(other.m_Head);
        m_Tail       = Move(other.m_Tail);
        m_Size       = other.m_Size;

        other.m_Head = nullptr;
        other.m_Tail = nullptr;
        other.m_Size = 0;

        // Change ownership
        for (PointerType current = m_Head; current; current = current->Hook.Next)
            current->Hook.Owner = this;

        return *this;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE constexpr bool
    IntrusiveRefList<T, HookType>::Empty() const
    {
        return !m_Head;
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE constexpr usize
    IntrusiveRefList<T, HookType>::Size() const
    {
        return m_Size;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE constexpr HookType::PointerType IntrusiveRefList<T, HookType>::Head()
    {
        return m_Head;
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE constexpr const HookType::PointerType
    IntrusiveRefList<T, HookType>::Head() const
    {
        return m_Head;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE constexpr HookType::PointerType IntrusiveRefList<T, HookType>::Tail()
    {
        return m_Tail;
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE constexpr const HookType::PointerType
    IntrusiveRefList<T, HookType>::Tail() const
    {
        return m_Tail;
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::begin()
    {
        return Iterator(m_Head, this);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::begin() const
    {
        return Iterator(m_Head, this);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::cbegin() const PM_NOEXCEPT
    {
        return Iterator(m_Head, this);
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::end()
    {
        return Iterator(nullptr, this);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::end() const
    {
        return Iterator(nullptr, this);
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::cend() const PM_NOEXCEPT
    {
        return Iterator(nullptr, this);
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rbegin()
    {
        return ReverseIterator(end());
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rbegin() const
    {
        return ReverseIterator(end());
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::crbegin() const PM_NOEXCEPT
    {
        return ReverseIterator(end());
    }

    template <typename T, typename HookType>
    PM_ALWAYS_INLINE IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rend()
    {
        return ReverseIterator(begin());
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::rend() const
    {
        return ReverseIterator(begin());
    }
    template <typename T, typename HookType>
    PM_ALWAYS_INLINE const IntrusiveRefList<T, HookType>::ReverseIterator
    IntrusiveRefList<T, HookType>::crend() const PM_NOEXCEPT
    {
        return ReverseIterator(begin());
    }

    template <typename T, typename HookType>
    constexpr void IntrusiveRefList<T, HookType>::Clear()
    {
        while (m_Head) HookType::Hook(m_Head).Unlink(m_Head);
    }

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::PushFront(HookType::PointerType node)
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
        return Iterator(node, this);
    }

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::PushBack(HookType::PointerType node)
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
        return Iterator(node, this);
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
    constexpr HookType::PointerType IntrusiveRefList<T, HookType>::PopFrontElement()
    {
        auto head = Head();
        PopFront();

        return head;
    }
    template <typename T, typename HookType>
    constexpr HookType::PointerType IntrusiveRefList<T, HookType>::PopBackElement()
    {
        auto tail = Tail();
        PopBack();

        return tail;
    }

    template <typename T, typename HookType>
    constexpr IntrusiveRefList<T, HookType>::Iterator
    IntrusiveRefList<T, HookType>::Erase(Iterator it)
    {
        if (!it.Current) return end();
        auto& hook = HookType::Hook(it.Current);
        if (hook.Owner != this) return end();

        PointerType next = hook.Next;
        hook.Unlink(it.Current);
        return Iterator(next, this);
    }
    template <typename T, typename HookType>
    constexpr void IntrusiveRefList<T, HookType>::Erase(HookType::PointerType node)
    {
        if (!node) return;
        auto& hook = HookType::Hook(node);
        if (hook.Owner != this) return;

        hook.Unlink(node);
    }
}; // namespace Prism
