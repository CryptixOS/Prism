/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T, typename HookType>
    class IntrusiveList;

    template <typename T>
    struct IntrusiveListHook
    {
        IntrusiveList<T, IntrusiveListHook<T>>* Owner    = nullptr;
        T*                                      Previous = nullptr;
        T*                                      Next     = nullptr;

        static IntrusiveListHook& Hook(T* node) { return node->Hook; }
        constexpr bool            IsLinked() const { return Owner; }
        void                      Unlink(T* self);
    };

    template <typename T, typename HookType = IntrusiveListHook<T>>
    class IntrusiveList
    {
      public:
        struct Iterator
        {
            T* Current;

            constexpr Iterator() = default;
            constexpr explicit Iterator(T* node)
                : Current(node)
            {
            }

            constexpr static Iterator UniversalEnd()
            {
                return Iterator(nullptr);
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
        using ReverseIterator = std::reverse_iterator<Iterator>;

        friend struct IntrusiveListHook<T>;
        constexpr IntrusiveList() = default;

        constexpr IntrusiveList(const IntrusiveList& other);
        constexpr IntrusiveList(IntrusiveList&& other);

        constexpr ~IntrusiveList();

        constexpr IntrusiveList& operator=(const IntrusiveList& other);
        constexpr IntrusiveList& operator=(IntrusiveList&& other);

        PM_ALWAYS_INLINE inline constexpr bool     Empty() const;
        PM_ALWAYS_INLINE inline constexpr usize    Size() const;

        PM_ALWAYS_INLINE inline constexpr T*       Head();
        PM_ALWAYS_INLINE inline constexpr const T* Head() const;

        PM_ALWAYS_INLINE inline constexpr T*       Tail();
        PM_ALWAYS_INLINE inline constexpr const T* Tail() const;

        PM_ALWAYS_INLINE inline Iterator           begin();
        PM_ALWAYS_INLINE inline const Iterator     begin() const;
        PM_ALWAYS_INLINE inline const Iterator     cbegin() const PM_NOEXCEPT;

        PM_ALWAYS_INLINE inline Iterator           end();
        PM_ALWAYS_INLINE inline const Iterator     end() const;
        PM_ALWAYS_INLINE inline const Iterator     cend() const PM_NOEXCEPT;

        PM_ALWAYS_INLINE inline ReverseIterator    rbegin();
        PM_ALWAYS_INLINE inline const ReverseIterator rbegin() const;
        PM_ALWAYS_INLINE inline const ReverseIterator
                                                crbegin() const PM_NOEXCEPT;

        PM_ALWAYS_INLINE inline ReverseIterator rend();
        PM_ALWAYS_INLINE inline const ReverseIterator rend() const;
        PM_ALWAYS_INLINE inline const ReverseIterator crend() const PM_NOEXCEPT;

        constexpr void                                Clear();

        template <typename... Args>
        inline constexpr Iterator EmplaceFront(Args&&... args);
        template <typename... Args>
        inline constexpr Iterator EmplaceBack(Args&&... args);

        inline constexpr Iterator PushFront(T* node);
        inline constexpr Iterator PushBack(T* node);

        inline constexpr void     PopFront();
        inline constexpr void     PopBack();

        inline constexpr T*       PopFrontElement();
        inline constexpr T*       PopBackElement();

        inline constexpr Iterator Erase(Iterator it);
        inline constexpr void     Erase(T* node);

      private:
        T*    m_Head = nullptr;
        T*    m_Tail = nullptr;
        usize m_Size = 0;
    };
}; // namespace Prism

#include <Prism/Containers/IntrusiveList.inl>

#if PRISM_TARGET_CRYPTIX == 1
using Prism::IntrusiveList;
using Prism::IntrusiveListHook;
#endif
