/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Containers/IntrusiveListV3.hpp>

namespace Prism::V3
{
    // IntrusiveListHook
    template <typename T, typename Pointer>
    IntrusiveListHook<T, Pointer>::~IntrusiveListHook()
    {
        if constexpr (Policy::IsOwning)
            assert(!IsLinked()
                   && "Destroying a node still owned by an IntrusiveList");
        else Detach();
    }

    template <typename T, typename Pointer>
    T* IntrusiveListHook<T, Pointer>::Detach()
    {
        StateType* owner = m_Owner;
        if (!owner) return nullptr;

        T* self = m_Previous ? owner->HookOf(m_Previous).m_Next : owner->Head;

        if (m_Previous) owner->HookOf(m_Previous).m_Next = m_Next;
        else owner->Head = m_Next;

        if (m_Next) owner->HookOf(m_Next).m_Previous = m_Previous;
        else owner->Tail = m_Previous;

        --owner->Size;
        m_Owner    = nullptr;
        m_Previous = nullptr;
        m_Next     = nullptr;

        return self;
    }

    template <typename T, typename Pointer>
    void IntrusiveListHook<T, Pointer>::Unlink()
    {
        // Detach fully before releasing: Release may destroy the node, and
        // with it this hook.
        Policy::Release(Detach());
    }

    // IntrusiveList
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::IntrusiveList(
        IntrusiveList&& other) PM_NOEXCEPT
    {
        AdoptAll(other.m_State.Head);
        other.m_State.Head = nullptr;
        other.m_State.Tail = nullptr;
        other.m_State.Size = 0;
    }

    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>&
    IntrusiveList<T, HookMember>::operator=(IntrusiveList&& other) PM_NOEXCEPT
    {
        if (this == &other) return *this;

        Clear();
        AdoptAll(other.m_State.Head);
        other.m_State.Head = nullptr;
        other.m_State.Tail = nullptr;
        other.m_State.Size = 0;

        return *this;
    }

    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::~IntrusiveList()
    {
        Clear();
    }

    template <typename T, auto HookMember>
    void IntrusiveList<T, HookMember>::AdoptAll(T* head)
    {
        assert(Empty());
        if (!head) return;

        // Re-own every node; `head` belongs to another list's state.
        auto* source  = HookOf(head).m_Owner;
        m_State.Head  = source->Head;
        m_State.Tail  = source->Tail;
        m_State.Size  = source->Size;

        for (T* node = head; node; node = HookOf(node).m_Next)
            HookOf(node).m_Owner = &m_State;
    }

    template <typename T, auto HookMember>
    constexpr bool IntrusiveList<T, HookMember>::Empty() const PM_NOEXCEPT
    {
        return m_State.Size == 0;
    }
    template <typename T, auto HookMember>
    constexpr usize IntrusiveList<T, HookMember>::Size() const PM_NOEXCEPT
    {
        return m_State.Size;
    }

    template <typename T, auto HookMember>
    constexpr T* IntrusiveList<T, HookMember>::Head() PM_NOEXCEPT
    {
        return m_State.Head;
    }
    template <typename T, auto HookMember>
    constexpr const T* IntrusiveList<T, HookMember>::Head() const PM_NOEXCEPT
    {
        return m_State.Head;
    }
    template <typename T, auto HookMember>
    constexpr T* IntrusiveList<T, HookMember>::Tail() PM_NOEXCEPT
    {
        return m_State.Tail;
    }
    template <typename T, auto HookMember>
    constexpr const T* IntrusiveList<T, HookMember>::Tail() const PM_NOEXCEPT
    {
        return m_State.Tail;
    }

    template <typename T, auto HookMember>
    constexpr bool IntrusiveList<T, HookMember>::Contains(const T& node) const
    {
        return HookOf(&node).m_Owner == &m_State;
    }

    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::begin() PM_NOEXCEPT
    {
        return Iterator(&m_State, m_State.Head);
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstIterator
    IntrusiveList<T, HookMember>::begin() const PM_NOEXCEPT
    {
        return ConstIterator(&m_State, m_State.Head);
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstIterator
    IntrusiveList<T, HookMember>::cbegin() const PM_NOEXCEPT
    {
        return begin();
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::end() PM_NOEXCEPT
    {
        return Iterator(&m_State, nullptr);
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstIterator
    IntrusiveList<T, HookMember>::end() const PM_NOEXCEPT
    {
        return ConstIterator(&m_State, nullptr);
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstIterator
    IntrusiveList<T, HookMember>::cend() const PM_NOEXCEPT
    {
        return end();
    }

    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ReverseIterator
    IntrusiveList<T, HookMember>::rbegin() PM_NOEXCEPT
    {
        return ReverseIterator(end());
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstReverseIterator
    IntrusiveList<T, HookMember>::rbegin() const PM_NOEXCEPT
    {
        return ConstReverseIterator(end());
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstReverseIterator
    IntrusiveList<T, HookMember>::crbegin() const PM_NOEXCEPT
    {
        return rbegin();
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ReverseIterator
    IntrusiveList<T, HookMember>::rend() PM_NOEXCEPT
    {
        return ReverseIterator(begin());
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstReverseIterator
    IntrusiveList<T, HookMember>::rend() const PM_NOEXCEPT
    {
        return ConstReverseIterator(begin());
    }
    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::ConstReverseIterator
    IntrusiveList<T, HookMember>::crend() const PM_NOEXCEPT
    {
        return rend();
    }

    template <typename T, auto HookMember>
    constexpr IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::IteratorTo(T& node)
    {
        assert(Contains(node));
        return Iterator(&m_State, &node);
    }

    template <typename T, auto HookMember>
    void IntrusiveList<T, HookMember>::Clear()
    {
        while (m_State.Head) PopFront();
    }

    template <typename T, auto HookMember>
    void IntrusiveList<T, HookMember>::Link(T* position, T* node)
    {
        auto& hook = HookOf(node);
        assert(!hook.IsLinked() && "Node is already linked into a list");

        T* previous     = position ? HookOf(position).m_Previous : m_State.Tail;

        hook.m_Owner    = &m_State;
        hook.m_Previous = previous;
        hook.m_Next     = position;

        if (previous) HookOf(previous).m_Next = node;
        else m_State.Head = node;

        if (position) HookOf(position).m_Previous = node;
        else m_State.Tail = node;

        ++m_State.Size;
    }

    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::PushFront(PointerType node)
    {
        return InsertBefore(begin(), Move(node));
    }
    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::PushBack(PointerType node)
    {
        return InsertBefore(end(), Move(node));
    }

    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::InsertBefore(ConstIterator position,
                                               PointerType   node)
    {
        assert(position.m_List == &m_State && "Iterator from another list");
        T* raw = Policy::Raw(node);
        assert(raw && "Pushing a null node");
        if (!raw) return end();

        // A node that's already linked is a bug. Leaving the caller's
        // ownership untouched means a misuse can't leak or double-free.
        if (HookOf(raw).IsLinked())
        {
            assert(!"Node is already linked into a list");
            return end();
        }

        Link(position.m_Current, Policy::Leak(Move(node)));
        return Iterator(&m_State, raw);
    }
    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::InsertAfter(ConstIterator position,
                                              PointerType   node)
    {
        T* next = position.m_Current ? HookOf(position.m_Current).m_Next
                                     : m_State.Head;
        return InsertBefore(ConstIterator(&m_State, next), Move(node));
    }

    template <typename T, auto HookMember>
    template <typename... Args>
        requires(IntrusiveList<T, HookMember>::Policy::IsOwning)
    IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::EmplaceFront(Args&&... args)
    {
        return PushFront(Policy::Adopt(new T(Forward<Args>(args)...)));
    }
    template <typename T, auto HookMember>
    template <typename... Args>
        requires(IntrusiveList<T, HookMember>::Policy::IsOwning)
    IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::EmplaceBack(Args&&... args)
    {
        return PushBack(Policy::Adopt(new T(Forward<Args>(args)...)));
    }

    template <typename T, auto HookMember>
    void IntrusiveList<T, HookMember>::PopFront()
    {
        if (m_State.Head) HookOf(m_State.Head).Unlink();
    }
    template <typename T, auto HookMember>
    void IntrusiveList<T, HookMember>::PopBack()
    {
        if (m_State.Tail) HookOf(m_State.Tail).Unlink();
    }

    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::PointerType
    IntrusiveList<T, HookMember>::TakeFront()
    {
        if (!m_State.Head) return Policy::Adopt(nullptr);
        return Take(*m_State.Head);
    }
    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::PointerType
    IntrusiveList<T, HookMember>::TakeBack()
    {
        if (!m_State.Tail) return Policy::Adopt(nullptr);
        return Take(*m_State.Tail);
    }
    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::PointerType
    IntrusiveList<T, HookMember>::Take(T& node)
    {
        if (!Contains(node))
        {
            assert(!"Node is not linked into this list");
            return Policy::Adopt(nullptr);
        }
        return Policy::Adopt(HookOf(&node).Detach());
    }

    template <typename T, auto HookMember>
    IntrusiveList<T, HookMember>::Iterator
    IntrusiveList<T, HookMember>::Erase(ConstIterator it)
    {
        T* node = it.m_Current;
        if (!node || !Contains(*node))
        {
            assert(!"Erasing an invalid iterator");
            return end();
        }

        T* next = HookOf(node).m_Next;
        HookOf(node).Unlink();
        return Iterator(&m_State, next);
    }
    template <typename T, auto HookMember>
    bool IntrusiveList<T, HookMember>::Erase(T& node)
    {
        if (!Contains(node)) return false;
        HookOf(&node).Unlink();
        return true;
    }

    template <typename T, auto HookMember>
    void IntrusiveList<T, HookMember>::Splice(IntrusiveList& other)
    {
        if (&other == this || other.Empty()) return;

        for (T* node = other.m_State.Head; node; node = HookOf(node).m_Next)
            HookOf(node).m_Owner = &m_State;

        if (m_State.Tail)
        {
            HookOf(m_State.Tail).m_Next            = other.m_State.Head;
            HookOf(other.m_State.Head).m_Previous = m_State.Tail;
        }
        else m_State.Head = other.m_State.Head;

        m_State.Tail       = other.m_State.Tail;
        m_State.Size      += other.m_State.Size;

        other.m_State.Head = nullptr;
        other.m_State.Tail = nullptr;
        other.m_State.Size = 0;
    }
} // namespace Prism::V3
