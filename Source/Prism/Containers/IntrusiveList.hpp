/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T, typename HookType>
    class IntrusiveList;

    template <typename T>
    struct IntrusiveListHook
    {
        T*                                      Previous = nullptr;
        T*                                      Next     = nullptr;
        IntrusiveList<T, IntrusiveListHook<T>>* Owner    = nullptr;

        static IntrusiveListHook& Hook(T* node) { return node->Hook; }
        constexpr bool            IsLinked() const { return Owner; }
        void                      Unlink(T* self);
    };

    template <typename T, typename HookType = IntrusiveListHook<T>>
    class IntrusiveList
    {
      public:
        friend class IntrusiveListHook<T>;
        constexpr IntrusiveList() = default;

        constexpr bool     Empty() const { return !m_Head; }

        constexpr T*       Head() { return m_Head; }
        constexpr const T* Head() const { return m_Head; }
        constexpr T*       Tail() { return m_Tail; }
        constexpr const T* Tail() const { return m_Tail; }

        constexpr void     PushFront(T* node);
        constexpr void     PushBack(T* node);
        constexpr void     PopFront();
        constexpr void     PopBack();

        constexpr void     Erase(T* node);
        constexpr void     Clear();

        struct Iterator
        {
            T* Current;

            constexpr Iterator() = default;
            constexpr explicit Iterator(T* node)
                : Current(node)
            {
            }

            constexpr T&        operator*() const { return *Current; }
            constexpr T*        operator->() const { return Current; }

            constexpr Iterator& operator++();
            constexpr Iterator  operator++(int);

            constexpr bool      operator==(const Iterator& other) const
            {
                return Current == other.Current;
            }
            constexpr bool operator!=(const Iterator& other) const
            {
                return Current != other.Current;
            }
        };

        constexpr Iterator begin() { return Iterator(m_Head); }
        constexpr Iterator end() { return Iterator(nullptr); }

        constexpr Iterator begin() const { return Iterator(m_Head); }
        constexpr Iterator end() const { return Iterator(nullptr); }

      private:
        T* m_Head = nullptr;
        T* m_Tail = nullptr;
    };
}; // namespace Prism

#include <Prism/Containers/IntrusiveList.inl>

#if PRISM_TARGET_CRYPTIX == 1
using Prism::IntrusiveList;
using Prism::IntrusiveListHook;
#endif
