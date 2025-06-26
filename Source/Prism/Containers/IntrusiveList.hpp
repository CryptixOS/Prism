/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/Types.hpp>

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
            ///> Current node.
            T* Current;

            /// @brief Default constructor.
            constexpr Iterator() = default;
            /**
             * @brief Constructs an iterator from a node.
             *
             * @param node Pointer to the node.
             */
            constexpr explicit Iterator(T* node)
                : Current(node)
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

        /// @brief Copy constructor.
        constexpr IntrusiveList(const IntrusiveList& other);
        /// @brief Move constructor.
        constexpr IntrusiveList(IntrusiveList&& other);

        /// @brief Destructor.
        constexpr ~IntrusiveList();

        /// @brief Copy assignment.
        constexpr IntrusiveList& operator=(const IntrusiveList& other);
        /// @brief Move assignment.
        constexpr IntrusiveList& operator=(IntrusiveList&& other);

        /// @brief Returns whether the list is empty.
        PM_ALWAYS_INLINE inline constexpr bool     Empty() const;
        /// @brief Returns the number of elements in the list.
        PM_ALWAYS_INLINE inline constexpr usize    Size() const;

        /// @brief Returns pointer to the head node.
        PM_ALWAYS_INLINE inline constexpr T*       Head();
        /// @brief Const-qualified version of Head().
        PM_ALWAYS_INLINE inline constexpr const T* Head() const;

        /// @brief Returns pointer to the tail node.
        PM_ALWAYS_INLINE inline constexpr T*       Tail();
        /// @brief Const-qualified version of Tail().
        PM_ALWAYS_INLINE inline constexpr const T* Tail() const;

        /// @name Forward Iterators
        /// @{

        PM_ALWAYS_INLINE inline Iterator           begin();
        PM_ALWAYS_INLINE inline const Iterator     begin() const;
        PM_ALWAYS_INLINE inline const Iterator     cbegin() const PM_NOEXCEPT;

        PM_ALWAYS_INLINE inline Iterator           end();
        PM_ALWAYS_INLINE inline const Iterator     end() const;
        PM_ALWAYS_INLINE inline const Iterator     cend() const PM_NOEXCEPT;

        /// @}

        /// @name Reverse Iterators
        /// @{

        PM_ALWAYS_INLINE inline ReverseIterator    rbegin();
        PM_ALWAYS_INLINE inline const ReverseIterator rbegin() const;
        PM_ALWAYS_INLINE inline const ReverseIterator
                                                crbegin() const PM_NOEXCEPT;

        PM_ALWAYS_INLINE inline ReverseIterator rend();
        PM_ALWAYS_INLINE inline const ReverseIterator rend() const;
        PM_ALWAYS_INLINE inline const ReverseIterator crend() const PM_NOEXCEPT;

        /// @}

        /**
         * @brief Removes all elements from the list.
         */
        constexpr void                                Clear();

        /**
         * @brief Constructs and inserts a node at the front.
         *
         * @tparam Args Constructor argument types.
         * @param args Arguments forwarded to node constructor.
         * @return Iterator Iterator to the inserted node.
         */
        template <typename... Args>
        inline constexpr Iterator EmplaceFront(Args&&... args);
        /**
         * @brief Constructs and inserts a node at the back.
         *
         * @tparam Args Constructor argument types.
         * @param args Arguments forwarded to node constructor.
         * @return Iterator Iterator to the inserted node.
         */
        template <typename... Args>
        inline constexpr Iterator EmplaceBack(Args&&... args);

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

#include <Prism/Containers/IntrusiveList.inl>

#if PRISM_TARGET_CRYPTIX == 1
using Prism::IntrusiveList;
using Prism::IntrusiveListHook;
#endif
