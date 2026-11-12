/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Containers.IntrusiveListV3;

export import Prism.Core.Iterator;
export import Prism.Memory.Ref;
export import Prism.Memory.Scope;

/**
 * Unified intrusive doubly-linked list.
 *
 * Replaces both Prism::IntrusiveList and Prism::IntrusiveRefList. It lives in
 * the Prism::V3 namespace so it can coexist with the old implementations until
 * they are removed; after that it can be renamed to IntrusiveList.hpp and the
 * V3 namespace dropped.
 *
 * The ownership policy is selected by the hook's Pointer parameter:
 *  - IntrusiveListHook<T>           (T*)       : non-owning, the list never
 *                                                frees nodes.
 *  - IntrusiveListHook<T, Ref<T>>   (Ref<T>)   : the list holds exactly one
 *                                                strong reference per node.
 *  - IntrusiveListHook<T, Scope<T>> (Scope<T>) : the list owns its nodes and
 *                                                deletes them on removal.
 *
 * Links are always raw T* pointers, so Ref-based lists never form reference
 * cycles and traversal never touches reference counts.
 */

export extern "C++"
{
    namespace Prism::V3
    {
        /**
         * @brief Describes how a list takes and gives back ownership of a node.
         *
         * Specialize it to support custom smart pointers. A policy has to provide:
         *  - PointerType                 : what users push and what Pop* returns
         *  - static T*  Raw(const P&)    : access the node without changing
         *                                  ownership
         *  - static T*  Leak(P&&)        : transfer ownership into the list
         *  - static P   Adopt(T*)        : transfer ownership back out of the list
         *  - static void Release(T*)     : drop the list's ownership
         *  - static constexpr bool IsOwning
         */
        template <typename Pointer>
        struct IntrusivePointerPolicy;

        template <typename T>
        struct IntrusivePointerPolicy<T*>
        {
            using ElementType = T;
            using PointerType = T*;
            static constexpr bool IsOwning = false;

            static constexpr T*   Raw(T* ptr) PM_NOEXCEPT { return ptr; }
            static constexpr T*   Leak(T* ptr) PM_NOEXCEPT { return ptr; }
            static constexpr T*   Adopt(T* ptr) PM_NOEXCEPT { return ptr; }
            static constexpr void Release(T*) PM_NOEXCEPT {}
        };

        template <typename T>
        struct IntrusivePointerPolicy<Ref<T>>
        {
            using ElementType = T;
            using PointerType = Ref<T>;
            static constexpr bool IsOwning = true;

            static T*      Raw(const Ref<T>& ptr) PM_NOEXCEPT { return ptr.Raw(); }
            static T*      Leak(Ref<T>&& ptr) PM_NOEXCEPT { return ptr.Leak(); }
            static Ref<T>  Adopt(T* ptr) PM_NOEXCEPT
            {
                return ptr ? Ref<T>(AdoptRef, ptr) : Ref<T>();
            }
            static void Release(T* ptr) PM_NOEXCEPT
            {
                if (ptr) ptr->Unref();
            }
        };

        template <typename T>
        struct IntrusivePointerPolicy<Scope<T>>
        {
            using ElementType = T;
            using PointerType = Scope<T>;
            static constexpr bool IsOwning = true;

            static T* Raw(const Scope<T>& ptr) PM_NOEXCEPT { return ptr.Raw(); }
            static T* Leak(Scope<T>&& ptr) PM_NOEXCEPT { return ptr.Release(); }
            static Scope<T> Adopt(T* ptr) PM_NOEXCEPT { return Scope<T>(ptr); }
            static void     Release(T* ptr) PM_NOEXCEPT { DefaultDelete<T>{}(ptr); }
        };

        template <typename T, typename Pointer = T*>
        class IntrusiveListHook;
        template <typename T, auto HookMember = &T::Hook>
        class IntrusiveList;

        namespace Details
        {
            /**
             * @brief State shared between a list and the hooks it links.
             *
             * Hooks point at this rather than at the list class, so a hook does
             * not need to know which member of T it is. That lets a hook unlink
             * itself (keeping Size() correct) without the caller passing `this`.
             */
            template <typename T, typename Pointer>
            struct IntrusiveListState
            {
                using HookType = IntrusiveListHook<T, Pointer>;

                T*    Head = nullptr;
                T*    Tail = nullptr;
                usize Size = 0;
                HookType& (*HookOf)(T*) = nullptr;
            };
        } // namespace Details

        /**
         * @brief Hook embedded in list elements for intrusive linking.
         *
         * Copying a node never copies its link state: the copy starts out
         * unlinked. A non-owning hook unlinks itself on destruction. An owning
         * hook asserts that it isn't linked, because a list still holds
         * ownership of the node.
         *
         * @tparam T Type of the node that contains this hook.
         * @tparam Pointer Ownership policy (T*, Ref<T>, Scope<T>, ...).
         */
        template <typename T, typename Pointer>
        class IntrusiveListHook
        {
          public:
            using ElementType = T;
            using Policy      = IntrusivePointerPolicy<Pointer>;
            using PointerType = typename Policy::PointerType;
            using StateType   = Details::IntrusiveListState<T, Pointer>;

            constexpr IntrusiveListHook() PM_NOEXCEPT = default;
            constexpr IntrusiveListHook(const IntrusiveListHook&) PM_NOEXCEPT {}
            constexpr IntrusiveListHook&
            operator=(const IntrusiveListHook&) PM_NOEXCEPT
            {
                return *this;
            }
            ~IntrusiveListHook();

            /// @brief Whether the node is currently part of a list.
            PM_NODISCARD constexpr bool IsLinked() const PM_NOEXCEPT
            {
                return m_Owner != nullptr;
            }
            /// @brief Next node in the list, or nullptr.
            PM_NODISCARD constexpr T* Next() const PM_NOEXCEPT { return m_Next; }
            /// @brief Previous node in the list, or nullptr.
            PM_NODISCARD constexpr T* Previous() const PM_NOEXCEPT
            {
                return m_Previous;
            }

            /**
             * @brief Removes the node from whatever list it's in.
             *
             * For owning policies the list's ownership is released, which may
             * destroy the node (and with it this hook), so don't touch the node
             * afterwards unless you hold your own reference.
             */
            void Unlink();

          private:
            template <typename U, auto HookMember>
            friend class IntrusiveList;

            /// Detaches the node and returns it without releasing ownership.
            T*         Detach();

            StateType* m_Owner    = nullptr;
            T*         m_Previous = nullptr;
            T*         m_Next     = nullptr;
        };

        template <typename T>
        using IntrusiveRefListHook = IntrusiveListHook<T, Ref<T>>;
        template <typename T>
        using IntrusiveOwningListHook = IntrusiveListHook<T, Scope<T>>;

        namespace Details
        {
            template <typename M>
            struct HookMemberTraits;
            template <typename T, typename Hook>
            struct HookMemberTraits<Hook T::*>
            {
                using ElementType = T;
                using HookType    = Hook;
            };
        } // namespace Details

        /**
         * @brief Intrusive doubly-linked list.
         *
         * @tparam T Node type.
         * @tparam HookMember Pointer to the hook member of T. Its type picks the
         * ownership policy. Defaults to &T::Hook.
         */
        template <typename T, auto HookMember>
        class IntrusiveList
        {
            using MemberTraits = Details::HookMemberTraits<decltype(HookMember)>;
            static_assert(IsSameV<typename MemberTraits::ElementType, T>,
                          "HookMember must be a member of T");

          public:
            using HookType    = typename MemberTraits::HookType;
            using Policy      = typename HookType::Policy;
            using PointerType = typename Policy::PointerType;
            using StateType   = typename HookType::StateType;
            using ValueType   = T;
            using SizeType    = usize;

            template <bool Const>
            class BasicIterator
            {
              public:
                using IteratorCategory = BidirectionalIteratorTag;
                using ValueType        = T;
                using DifferenceType   = ptrdiff;
                using Pointer          = ConditionalType<Const, const T*, T*>;
                using Reference        = ConditionalType<Const, const T&, T&>;

                constexpr BasicIterator() = default;
                constexpr BasicIterator(const StateType* list, T* node)
                    : m_List(list)
                    , m_Current(node)
                {
                }
                /// Non-const to const conversion.
                template <bool OtherConst>
                    requires(Const && !OtherConst)
                constexpr BasicIterator(const BasicIterator<OtherConst>& other)
                    : m_List(other.m_List)
                    , m_Current(other.m_Current)
                {
                }

                constexpr Reference operator*() const
                {
                    assert(m_Current && "Dereferencing end iterator");
                    return *m_Current;
                }
                constexpr Pointer operator->() const { return m_Current; }
                /// @brief The node this iterator points to (nullptr for end).
                constexpr Pointer Node() const { return m_Current; }

                constexpr BasicIterator& operator++()
                {
                    if (m_Current) m_Current = HookOf(m_Current).Next();
                    return *this;
                }
                constexpr BasicIterator operator++(int)
                {
                    auto copy = *this;
                    ++*this;
                    return copy;
                }
                constexpr BasicIterator& operator--()
                {
                    m_Current = m_Current ? HookOf(m_Current).Previous()
                                          : (m_List ? m_List->Tail : nullptr);
                    return *this;
                }
                constexpr BasicIterator operator--(int)
                {
                    auto copy = *this;
                    --*this;
                    return copy;
                }

                constexpr bool operator==(const BasicIterator& other) const
                {
                    return m_Current == other.m_Current;
                }
                constexpr bool operator!=(const BasicIterator& other) const
                {
                    return m_Current != other.m_Current;
                }

              private:
                friend class IntrusiveList;
                template <bool>
                friend class BasicIterator;

                const StateType* m_List    = nullptr;
                T*               m_Current = nullptr;
            };

            using Iterator             = BasicIterator<false>;
            using ConstIterator        = BasicIterator<true>;
            using ReverseIterator      = ::Prism::ReverseIterator<Iterator>;
            using ConstReverseIterator = ::Prism::ReverseIterator<ConstIterator>;

            constexpr IntrusiveList() PM_NOEXCEPT = default;
            constexpr IntrusiveList(IntrusiveList&& other) PM_NOEXCEPT;
            IntrusiveList(const IntrusiveList&)            = delete;
            IntrusiveList& operator=(const IntrusiveList&) = delete;
            constexpr IntrusiveList& operator=(IntrusiveList&& other) PM_NOEXCEPT;
            ~IntrusiveList();

            PM_NODISCARD constexpr bool  Empty() const PM_NOEXCEPT;
            PM_NODISCARD constexpr usize Size() const PM_NOEXCEPT;

            /// @brief First node, or nullptr when empty.
            PM_NODISCARD constexpr T*       Head() PM_NOEXCEPT;
            PM_NODISCARD constexpr const T* Head() const PM_NOEXCEPT;
            /// @brief Last node, or nullptr when empty.
            PM_NODISCARD constexpr T*       Tail() PM_NOEXCEPT;
            PM_NODISCARD constexpr const T* Tail() const PM_NOEXCEPT;

            /// @brief Whether `node` is linked into *this* list (O(1)).
            PM_NODISCARD constexpr bool     Contains(const T& node) const;

            /// @name Iterators
            /// @{
            constexpr Iterator              begin() PM_NOEXCEPT;
            constexpr ConstIterator         begin() const PM_NOEXCEPT;
            constexpr ConstIterator         cbegin() const PM_NOEXCEPT;
            constexpr Iterator              end() PM_NOEXCEPT;
            constexpr ConstIterator         end() const PM_NOEXCEPT;
            constexpr ConstIterator         cend() const PM_NOEXCEPT;

            constexpr ReverseIterator       rbegin() PM_NOEXCEPT;
            constexpr ConstReverseIterator  rbegin() const PM_NOEXCEPT;
            constexpr ConstReverseIterator  crbegin() const PM_NOEXCEPT;
            constexpr ReverseIterator       rend() PM_NOEXCEPT;
            constexpr ConstReverseIterator  rend() const PM_NOEXCEPT;
            constexpr ConstReverseIterator  crend() const PM_NOEXCEPT;
            /// @}

            /// @brief Iterator to `node`, which must be linked into this list.
            constexpr Iterator              IteratorTo(T& node);

            /// @brief Unlinks every node, releasing ownership (owning policies).
            void                            Clear();

            /// @brief Links `node` at the front. It must not be linked anywhere.
            Iterator                        PushFront(PointerType node);
            /// @brief Links `node` at the back. It must not be linked anywhere.
            Iterator                        PushBack(PointerType node);
            /// @brief Links `node` before `position` (end() appends).
            Iterator InsertBefore(ConstIterator position, PointerType node);
            /// @brief Links `node` after `position` (end() prepends).
            Iterator InsertAfter(ConstIterator position, PointerType node);

            /**
             * @brief Creates a node and links it. Only for owning policies, since
             * a non-owning list would leak the allocation.
             */
            template <typename... Args>
                requires(Policy::IsOwning)
            Iterator EmplaceFront(Args&&... args);
            template <typename... Args>
                requires(Policy::IsOwning)
            Iterator    EmplaceBack(Args&&... args);

            /// @brief Unlinks the front node, releasing ownership. No-op if empty.
            void        PopFront();
            /// @brief Unlinks the back node, releasing ownership. No-op if empty.
            void        PopBack();
            /// @brief Unlinks and returns the front node (null/empty if empty).
            PointerType TakeFront();
            /// @brief Unlinks and returns the back node (null/empty if empty).
            PointerType TakeBack();
            /// @brief Unlinks `node` and hands its ownership back to the caller.
            PointerType Take(T& node);

            /// @brief Compatibility alias for TakeFront().
            PointerType PopFrontElement() { return TakeFront(); }
            /// @brief Compatibility alias for TakeBack().
            PointerType PopBackElement() { return TakeBack(); }

            /**
             * @brief Unlinks the node at `it` and releases ownership.
             * @return Iterator to the following node.
             */
            Iterator    Erase(ConstIterator it);
            /**
             * @brief Unlinks `node` if it belongs to this list.
             * @return true if the node was removed.
             */
            bool        Erase(T& node);
            bool        Erase(T* node) { return node && Erase(*node); }

            /// @brief Moves every node of `other` to the end of this list (O(n)
            /// because each hook's owner has to be updated).
            void        Splice(IntrusiveList& other);

          private:
            static HookType& HookOf(T* node) { return node->*HookMember; }
            static const HookType& HookOf(const T* node)
            {
                return node->*HookMember;
            }

            void      Link(T* position, T* node);
            void      AdoptAll(T* head);

            StateType m_State{nullptr, nullptr, 0, &IntrusiveList::HookOf};
        };
    } // namespace Prism::V3


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
}
