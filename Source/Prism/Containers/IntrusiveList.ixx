/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Containers.IntrusiveList;

export import Prism.Core.Iterator;

/**
 * @deprecated Superseded by Prism::V3::IntrusiveList in
 * <Prism/Containers/IntrusiveListV3.hpp>. Nothing in Prism uses this
 * header any more; it is kept only until it is deleted.
 */

export extern "C++"
{
    namespace Prism
    {
        template <typename T, typename HookType>
        class IntrusiveList;

        /**
         * @brief Hook structure embedded in list elements for intrusive linking.
         *
         * @tparam T Type of the node that contains this hook.
         */
        template <typename T>
        struct IntrusiveListHook
        {
            IntrusiveList<T, IntrusiveListHook<T>>* Owner    = nullptr;
            T*                                      Previous = nullptr;
            T*                                      Next     = nullptr;

            /**
             * @brief Access the hook from the containing node.
             *
             * @param node Pointer to node.
             * @return IntrusiveListHook& Reference to the hook.
             */
            static IntrusiveListHook& Hook(T* node) { return node->Hook; }
            /**
             * @brief Check if the node is currently part of a list.
             *
             * @return true If linked.
             * @return false If not linked.
             */
            constexpr bool            IsLinked() const { return Owner; }
            /**
             * @brief Unlinks the node from its list.
             *
             * @param self Pointer to the node.
             */
            void                      Unlink(T* self);
        };

        /**
         * @brief Doubly-linked list implementation using intrusive hooks.
         *
         * @tparam T Node type.
         * @tparam HookType Type of the intrusive hook (defaults to
         * IntrusiveListHook<T>).
         */
        template <typename T, typename HookType = IntrusiveListHook<T>>
        class IntrusiveList
        {
          public:
            /**
             * @brief Forward iterator for IntrusiveList.
             */
            struct Iterator
            {
                using IteratorCategory = BidirectionalIteratorTag;
                using ValueType        = T;
                using DifferenceType   = ptrdiff;
                using Pointer          = T*;
                using Reference        = T&;

                ///> Current node.
                T*                   Current = nullptr;
                ///> Owning list, needed to step back from end().
                const IntrusiveList* List    = nullptr;

                /// @brief Default constructor.
                constexpr Iterator() = default;
                /**
                 * @brief Constructs an iterator from a node.
                 *
                 * @param node Pointer to the node.
                 */
                constexpr explicit Iterator(T* node, const IntrusiveList* list
                                                     = nullptr)
                    : Current(node)
                    , List(list)
                {
                }

                /**
                 * @brief Special end iterator.
                 *
                 * @return Iterator The end iterator.
                 */
                constexpr static Iterator UniversalEnd()
                {
                    return Iterator(nullptr);
                }

                /// @brief Dereference operator.
                constexpr T&        operator*() const { return *Current; }
                /// @brief Arrow operator.
                constexpr T*        operator->() const { return Current; }

                /// @brief Prefix increment.
                constexpr Iterator& operator++();
                /// @brief Postfix increment.
                constexpr Iterator  operator++(int);
                /// @brief Prefix decrement (from end() goes to the tail).
                constexpr Iterator& operator--();
                /// @brief Postfix decrement.
                constexpr Iterator  operator--(int);

                /// @brief Equality comparison.
                constexpr bool      operator==(const Iterator& other) const
                {
                    return Current == other.Current;
                }
                /// @brief Inequality comparison.
                constexpr bool operator!=(const Iterator& other) const
                {
                    return Current != other.Current;
                }
            };
            /// @brief Reverse iterator type.
            using ReverseIterator = ::Prism::ReverseIterator<Iterator>;

            friend struct IntrusiveListHook<T>;
            /// @brief Default constructor.
            constexpr IntrusiveList() = default;

            /// @brief Not copyable: an intrusive list does not own its nodes.
            IntrusiveList(const IntrusiveList& other) = delete;
            /// @brief Move constructor.
            constexpr IntrusiveList(IntrusiveList&& other);

            /// @brief Destructor.
            constexpr ~IntrusiveList();

            /// @brief Not copyable: an intrusive list does not own its nodes.
            IntrusiveList& operator=(const IntrusiveList& other) = delete;
            /// @brief Move assignment.
            constexpr IntrusiveList& operator=(IntrusiveList&& other);

            /// @brief Returns whether the list is empty.
            PM_ALWAYS_INLINE constexpr bool     Empty() const;
            /// @brief Returns the number of elements in the list.
            PM_ALWAYS_INLINE constexpr usize    Size() const;

            /// @brief Returns pointer to the head node.
            PM_ALWAYS_INLINE constexpr T*       Head();
            /// @brief Const-qualified version of Head().
            PM_ALWAYS_INLINE constexpr const T* Head() const;

            /// @brief Returns pointer to the tail node.
            PM_ALWAYS_INLINE constexpr T*       Tail();
            /// @brief Const-qualified version of Tail().
            PM_ALWAYS_INLINE constexpr const T* Tail() const;

            /// @name Forward Iterators
            /// @{

            PM_ALWAYS_INLINE Iterator           begin();
            PM_ALWAYS_INLINE const Iterator     begin() const;
            PM_ALWAYS_INLINE const Iterator     cbegin() const PM_NOEXCEPT;

            PM_ALWAYS_INLINE Iterator           end();
            PM_ALWAYS_INLINE const Iterator     end() const;
            PM_ALWAYS_INLINE const Iterator     cend() const PM_NOEXCEPT;

            /// @}

            /// @name Reverse Iterators
            /// @{

            PM_ALWAYS_INLINE ReverseIterator    rbegin();
            PM_ALWAYS_INLINE const ReverseIterator rbegin() const;
            PM_ALWAYS_INLINE const ReverseIterator
                                                    crbegin() const PM_NOEXCEPT;

            PM_ALWAYS_INLINE ReverseIterator rend();
            PM_ALWAYS_INLINE const ReverseIterator rend() const;
            PM_ALWAYS_INLINE const ReverseIterator crend() const PM_NOEXCEPT;

            /// @}

            /**
             * @brief Removes all elements from the list.
             */
            constexpr void                                Clear();


            /**
             * @brief Inserts a node at the front of the list.
             *
             * @param node Pointer to the node.
             * @return Iterator Iterator to the inserted node.
             */
            inline constexpr Iterator PushFront(T* node);
            /**
             * @brief Inserts a node at the back of the list.
             *
             * @param node Pointer to the node.
             * @return Iterator Iterator to the inserted node.
             */
            inline constexpr Iterator PushBack(T* node);

            /**
             * @brief Removes the front node from the list.
             */
            inline constexpr void     PopFront();
            /**
             * @brief Removes the back node from the list.
             */
            inline constexpr void     PopBack();

            /**
             * @brief Removes and returns the front node.
             *
             * @return T* Pointer to the removed node.
             */
            inline constexpr T*       PopFrontElement();
            /**
             * @brief Removes and returns the back node.
             *
             * @return T* Pointer to the removed node.
             */
            inline constexpr T*       PopBackElement();

            /**
             * @brief Removes the node pointed to by an iterator.
             *
             * @param it Iterator to the node to erase.
             * @return Iterator Iterator to the next node.
             */
            inline constexpr Iterator Erase(Iterator it);
            /**
             * @brief Removes a specific node from the list.
             *
             * @param node Pointer to the node.
             */
            inline constexpr void     Erase(T* node);

          private:
            ///> Pointer to the first element.
            T*    m_Head = nullptr;
            ///> Pointer to the last element.
            T*    m_Tail = nullptr;
            ///> Number of elements in the list.
            usize m_Size = 0;
        };
    }; // namespace Prism


    namespace Prism
    {
        // IntrusiveListHook
        template <typename T>
        void IntrusiveListHook<T>::Unlink([[maybe_unused]] T* self)
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

        // IntrusiveList::Iterator
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

        template <typename T, typename HookType>
        constexpr IntrusiveList<T, HookType>::Iterator&
        IntrusiveList<T, HookType>::Iterator::operator--()
        {
            if (Current) Current = HookType::Hook(Current).Previous;
            else if (List) Current = List->m_Tail;

            return *this;
        }
        template <typename T, typename HookType>
        constexpr IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::Iterator::operator--(int)
        {
            Iterator temp = *this;
            --(*this);

            return temp;
        }

        // IntrusiveList
        template <typename T, typename HookType>
        inline constexpr IntrusiveList<T, HookType>::IntrusiveList(
            IntrusiveList&& other)
        {
            *this = Move(other);
        }

        template <typename T, typename HookType>
        inline constexpr IntrusiveList<T, HookType>::~IntrusiveList()
        {
            Clear();
        }

        template <typename T, typename HookType>
        constexpr IntrusiveList<T, HookType>&
        IntrusiveList<T, HookType>::operator=(IntrusiveList&& other)
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
            for (T* current = m_Head; current; current = current->Hook.Next)
                current->Hook.Owner = this;

            return *this;
        }

        template <typename T, typename HookType>
        PM_ALWAYS_INLINE constexpr bool
        IntrusiveList<T, HookType>::Empty() const
        {
            return !m_Head;
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE constexpr usize
        IntrusiveList<T, HookType>::Size() const
        {
            return m_Size;
        }

        template <typename T, typename HookType>
        PM_ALWAYS_INLINE constexpr T* IntrusiveList<T, HookType>::Head()
        {
            return m_Head;
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE constexpr const T*
        IntrusiveList<T, HookType>::Head() const
        {
            return m_Head;
        }

        template <typename T, typename HookType>
        PM_ALWAYS_INLINE constexpr T* IntrusiveList<T, HookType>::Tail()
        {
            return m_Tail;
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE constexpr const T*
        IntrusiveList<T, HookType>::Tail() const
        {
            return m_Tail;
        }

        template <typename T, typename HookType>
        PM_ALWAYS_INLINE IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::begin()
        {
            return Iterator(m_Head, this);
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::begin() const
        {
            return Iterator(m_Head, this);
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::cbegin() const PM_NOEXCEPT
        {
            return Iterator(m_Head, this);
        }

        template <typename T, typename HookType>
        PM_ALWAYS_INLINE IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::end()
        {
            return Iterator(nullptr, this);
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::end() const
        {
            return Iterator(nullptr, this);
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::cend() const PM_NOEXCEPT
        {
            return Iterator(nullptr, this);
        }

        template <typename T, typename HookType>
        PM_ALWAYS_INLINE IntrusiveList<T, HookType>::ReverseIterator
        IntrusiveList<T, HookType>::rbegin()
        {
            return ReverseIterator(end());
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::ReverseIterator
        IntrusiveList<T, HookType>::rbegin() const
        {
            return ReverseIterator(end());
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::ReverseIterator
        IntrusiveList<T, HookType>::crbegin() const PM_NOEXCEPT
        {
            return ReverseIterator(end());
        }

        template <typename T, typename HookType>
        PM_ALWAYS_INLINE IntrusiveList<T, HookType>::ReverseIterator
        IntrusiveList<T, HookType>::rend()
        {
            return ReverseIterator(begin());
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::ReverseIterator
        IntrusiveList<T, HookType>::rend() const
        {
            return ReverseIterator(begin());
        }
        template <typename T, typename HookType>
        PM_ALWAYS_INLINE const IntrusiveList<T, HookType>::ReverseIterator
        IntrusiveList<T, HookType>::crend() const PM_NOEXCEPT
        {
            return ReverseIterator(begin());
        }

        template <typename T, typename HookType>
        constexpr void IntrusiveList<T, HookType>::Clear()
        {
            while (m_Head) HookType::Hook(m_Head).Unlink(m_Head);
        }

        template <typename T, typename HookType>
        constexpr IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::PushFront(T* node)
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
        constexpr IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::PushBack(T* node)
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
        constexpr T* IntrusiveList<T, HookType>::PopFrontElement()
        {
            auto head = Head();
            PopFront();

            return head;
        }
        template <typename T, typename HookType>
        constexpr T* IntrusiveList<T, HookType>::PopBackElement()
        {
            auto tail = Tail();
            PopBack();

            return tail;
        }

        template <typename T, typename HookType>
        constexpr IntrusiveList<T, HookType>::Iterator
        IntrusiveList<T, HookType>::Erase(Iterator it)
        {
            if (!it.Current) return end();
            auto& hook = HookType::Hook(it.Current);
            if (hook.Owner != this) return end();

            T* next = hook.Next;
            hook.Unlink(it.Current);
            return Iterator(next, this);
        }
        template <typename T, typename HookType>
        constexpr void IntrusiveList<T, HookType>::Erase(T* node)
        {
            if (!node) return;
            auto& hook = HookType::Hook(node);
            if (hook.Owner != this) return;

            hook.Unlink(node);
        }
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::IntrusiveList;
    using Prism::IntrusiveListHook;
#endif
}
