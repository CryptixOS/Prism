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

#include <Prism/Memory/Ref.hpp>

namespace Prism
{
    template <typename T>
    struct PointerTraits
    {
        using Type = T;

        static Type Raw(Type addr) { return addr; }
        // static const Type Raw(const Type&  addr) { return addr; }
        static bool IsNull(Type addr) { return addr == nullptr; }
    };
    template <typename T>
    struct PointerTraits<Ref<T>>
    {
        using Type = Ref<T>;

        static T*       Raw(Type& addr) { return addr.Raw(); }
        static const T* Raw(const Type& addr) { return addr.Raw(); }
        static bool     IsNull(Type& addr) { return !addr; }
    };

    template <typename T, typename HookType>
    class IntrusiveRefList;

    /**
     * @brief Hook structure embedded in list elements for intrusive linking.
     *
     * @tparam T Type of the node that contains this hook.
     */
    template <typename T, typename Pointer = Ref<T>>
    struct IntrusiveRefListHook
    {
        using Type        = Pointer;
        using PointerType = typename PointerTraits<Pointer>::Type;

        IntrusiveRefList<T, IntrusiveRefListHook<T, Pointer>>* Owner = nullptr;
        PointerType                  Previous                        = nullptr;
        PointerType                  Next                            = nullptr;

        /**
         * @brief Access the hook from the containing node.
         *
         * @param node Pointer to node.
         * @return IntrusiveRefListHook& Reference to the hook.
         */
        static IntrusiveRefListHook& Hook(PointerType node)
        {
            return PointerTraits<PointerType>::Raw(node)->Hook;
        }
        /**
         * @brief Check if the node is currently part of a list.
         *
         * @return true If linked.
         * @return false If not linked.
         */
        constexpr bool IsLinked() const { return Owner; }
        /**
         * @brief Unlinks the node from its list.
         *
         * @param self Pointer to the node.
         */
        void           Unlink(PointerType self);
    };

    /**
     * @brief Doubly-linked list implementation using intrusive hooks.
     *
     * @tparam T Node type.
     * @tparam HookType Type of the intrusive hook (defaults to
     * IntrusiveRefListHook<T>).
     */
    template <typename T, typename HookType = IntrusiveRefListHook<T>>
    class IntrusiveRefList
    {
      public:
        using PointerType = HookType::PointerType;

        /**
         * @brief Forward iterator for IntrusiveRefList.
         */
        struct Iterator
        {
            ///> Current node.
            PointerType Current;

            /// @brief Default constructor.
            constexpr Iterator() = default;
            /**
             * @brief Constructs an iterator from a node.
             *
             * @param node Pointer to the node.
             */
            constexpr explicit Iterator(PointerType node)
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
            constexpr PointerType operator*() { return Current; }
            /// @brief Arrow operator.
            constexpr T&          operator->() { return *Current; }
            constexpr const T&    operator->() const { return *Current; }

            /// @brief Prefix increment.
            constexpr Iterator&   operator++();
            /// @brief Postfix increment.
            constexpr Iterator    operator++(int);

            /// @brief Equality comparison.
            constexpr bool        operator==(const Iterator& other) const
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

        friend struct IntrusiveRefListHook<T, typename HookType::Type>;
        /// @brief Default constructor.
        constexpr IntrusiveRefList() = default;

        /// @brief Copy constructor.
        constexpr IntrusiveRefList(const IntrusiveRefList& other);
        /// @brief Move constructor.
        constexpr IntrusiveRefList(IntrusiveRefList&& other);

        /// @brief Destructor.
        constexpr ~IntrusiveRefList();

        /// @brief Copy assignment.
        constexpr IntrusiveRefList& operator=(const IntrusiveRefList& other);
        /// @brief Move assignment.
        constexpr IntrusiveRefList& operator=(IntrusiveRefList&& other);

        /// @brief Returns whether the list is empty.
        PM_ALWAYS_INLINE constexpr bool              Empty() const;
        /// @brief Returns the number of elements in the list.
        PM_ALWAYS_INLINE constexpr usize             Size() const;

        /// @brief Returns pointer to the head node.
        PM_ALWAYS_INLINE constexpr PointerType       Head();
        /// @brief Const-qualified version of Head().
        PM_ALWAYS_INLINE constexpr const PointerType Head() const;

        /// @brief Returns pointer to the tail node.
        PM_ALWAYS_INLINE constexpr PointerType       Tail();
        /// @brief Const-qualified version of Tail().
        PM_ALWAYS_INLINE constexpr const PointerType Tail() const;

        /// @name Forward Iterators
        /// @{

        PM_ALWAYS_INLINE Iterator                    begin();
        PM_ALWAYS_INLINE const Iterator              begin() const;
        PM_ALWAYS_INLINE const Iterator              cbegin() const PM_NOEXCEPT;

        PM_ALWAYS_INLINE Iterator                    end();
        PM_ALWAYS_INLINE const Iterator              end() const;
        PM_ALWAYS_INLINE const Iterator              cend() const PM_NOEXCEPT;

        /// @}

        /// @name Reverse Iterators
        /// @{

        PM_ALWAYS_INLINE ReverseIterator             rbegin();
        PM_ALWAYS_INLINE const ReverseIterator       rbegin() const;
        PM_ALWAYS_INLINE const ReverseIterator crbegin() const PM_NOEXCEPT;

        PM_ALWAYS_INLINE ReverseIterator       rend();
        PM_ALWAYS_INLINE const ReverseIterator rend() const;
        PM_ALWAYS_INLINE const ReverseIterator crend() const PM_NOEXCEPT;

        /// @}

        /**
         * @brief Removes all elements from the list.
         */
        constexpr void                         Clear();

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
        inline constexpr Iterator    EmplaceBack(Args&&... args);

        /**
         * @brief Inserts a node at the front of the list.
         *
         * @param node Pointer to the node.
         * @return Iterator Iterator to the inserted node.
         */
        inline constexpr Iterator    PushFront(PointerType node);
        /**
         * @brief Inserts a node at the back of the list.
         *
         * @param node Pointer to the node.
         * @return Iterator Iterator to the inserted node.
         */
        inline constexpr Iterator    PushBack(PointerType node);

        /**
         * @brief Removes the front node from the list.
         */
        inline constexpr void        PopFront();
        /**
         * @brief Removes the back node from the list.
         */
        inline constexpr void        PopBack();

        /**
         * @brief Removes and returns the front node.
         *
         * @return PointerType Pointer to the removed node.
         */
        inline constexpr PointerType PopFrontElement();
        /**
         * @brief Removes and returns the back node.
         *
         * @return PointerType Pointer to the removed node.
         */
        inline constexpr PointerType PopBackElement();

        /**
         * @brief Removes the node pointed to by an iterator.
         *
         * @param it Iterator to the node to erase.
         * @return Iterator Iterator to the next node.
         */
        inline constexpr Iterator    Erase(Iterator it);
        /**
         * @brief Removes a specific node from the list.
         *
         * @param node Pointer to the node.
         */
        inline constexpr void        Erase(PointerType node);

      private:
        ///> Pointer to the first element.
        PointerType m_Head = nullptr;
        ///> Pointer to the last element.
        PointerType m_Tail = nullptr;
        ///> Number of elements in the list.
        usize       m_Size = 0;
    };
}; // namespace Prism

#include <Prism/Containers/IntrusiveRefList.inl>

#if PRISM_TARGET_CRYPTIX != 0
using Prism::IntrusiveRefList;
using Prism::IntrusiveRefListHook;
#endif
