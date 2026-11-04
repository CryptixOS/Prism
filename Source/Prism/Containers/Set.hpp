/*
 * Created by v1tr10l7 on 25.12.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/KeyValuePair.hpp>
#include <Prism/Containers/RedBlackTree.hpp>
#include <Prism/Core/Core.hpp>
#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/NonMovable.hpp>
#include <Prism/Utility/Compare.hpp>

PRISM_EXPORT namespace Prism
{
    template <typename Key, typename Compare = Less<Key>>
    class Set : public NonCopyable<Set<Key, Compare>>,
                public NonMovable<Set<Key, Compare>>
    {
      public:
        // Member types
        using KeyType        = Key;
        using ValueType      = Key;
        using SizeType       = usize;
        using DifferenceType = isize;
        using KeyCompare     = Compare;
        using ValueCompare   = Compare;

        using Reference      = ValueType&;
        using ConstReference = const ValueType&;

        using Pointer        = ValueType*;
        using ConstPointer   = const ValueType*;

        using TreeType       = RedBlackTree<Key, Key>;

        template <bool IsConst>
        struct BaseIterator;
        template <bool IsConst>
        struct BaseIterator
        {
            using ValueType        = Key;
            using ReferenceType    = const Key&;
            using PointerType      = const Key*;
            using DifferenceType   = isize;
            using IteratorCategory = BidirectionalIteratorTag;
            using UnderlyingIterator
                = ConditionalType<IsConst, typename TreeType::ConstIterator,
                                  typename TreeType::Iterator>;

            UnderlyingIterator Implementation;

            BaseIterator() = default;
            explicit BaseIterator(UnderlyingIterator it)
                : Implementation(it)
            {
            }

            template <bool B = IsConst, EnableIfType<B, i32> = 0>
            BaseIterator(const BaseIterator<false>& other)
                : Implementation(other.Implementation)
            {
            }

            ReferenceType operator*() const { return Implementation->Key; }
            PointerType   operator->() const { return &Implementation->Key; }

            BaseIterator& operator++()
            {
                ++Implementation;
                return *this;
            }
            BaseIterator operator++(int)
            {
                BaseIterator temp = *this;
                ++Implementation;
                return temp;
            }

            BaseIterator& operator--()
            {
                --Implementation;
                return *this;
            }
            BaseIterator operator--(int)
            {
                BaseIterator temp = *this;
                --Implementation;
                return temp;
            }

            constexpr bool operator==(const BaseIterator& other) const
            {
                return Implementation == other.Implementation;
            }
            constexpr bool operator!=(const BaseIterator& other) const
            {
                return Implementation != other.Implementation;
            }

            constexpr operator typename TreeType::Node*() const
            {
                return static_cast<typename TreeType::Node*>(Implementation);
            }
        };
        using Iterator             = BaseIterator<false>;
        using ConstIterator        = BaseIterator<true>;
        using ReverseIterator      = Prism::ReverseIterator<Iterator>;
        using ConstReverseIterator = Prism::ReverseIterator<ConstIterator>;

        struct NodeType
        {
            typename TreeType::Node* Node = nullptr;

            NodeType()                    = default;
            explicit NodeType(typename TreeType::Node* node)
                : Node(node)
            {
            }

            bool             Empty() const { return Node == nullptr; }
            explicit         operator bool() const { return Node != nullptr; }

            ValueType&       Value() { return Node->Key(); }
            const ValueType& Value() const { return Node->Key(); }
        };
        struct InsertReturnValue
        {
            Iterator Position;
            bool     Inserted;
            NodeType Node;
        };

        Set() = default;
        Set(const Set& other) { m_Tree = other.m_Tree; }
        Set(Set&& other) { m_Tree = Move(other.m_Tree); }

        explicit Set(const Compare& comp)
            : m_Compare(comp)
        {
        }
        template <typename It>
        Set(It first, It last, const Compare& comp = Compare())
            : m_Compare(comp)
        {
            for (auto it = first; it != last; it = Next(it)) Insert(*it);
        }
        Set(InitializerList<ValueType> ilist, const Compare& comp = Compare())
            : m_Compare(comp)
        {
            Insert(ilist.begin(), ilist.end());
        }

        constexpr Set& operator=(const Set& other)
        {
            m_Tree = other.m_Tree;
            return *this;
        }
        constexpr Set& operator=(Set&& other)
        {
            m_Tree = Move(other.m_Tree);
            return *this;
        }

        constexpr bool     Empty() const { return m_Tree.IsEmpty(); }
        constexpr SizeType Size() const { return m_Tree.GetSize(); }

        constexpr SizeType MaxSize() const
        {
            return NumericLimits<SizeType>::Max();
        }

        void                                   Clear() { m_Tree.Clear(); }

        constexpr KeyValuePair<Iterator, bool> Insert(const ValueType& value)
        {
            auto it = m_Tree.Find(value);
            if (it != m_Tree.end()) return {it, false};

            return {m_Tree.Insert(value, const_cast<ValueType&>(value)), true};
        }
        constexpr KeyValuePair<Iterator, bool> Insert(ValueType&& value)
        {
            auto treeIt = m_Tree.Insert(value, Move(value));
            return {Iterator(treeIt), true};
        }
        constexpr Iterator Insert(ConstIterator, const ValueType& value)
        {
            auto result = Insert(value);
            return result.First;
        }
        constexpr Iterator Insert(ConstIterator, ValueType&& value)
        {
            auto result = Insert(Move(value));
            return result.First;
        }
        template <typename InputIt>
        constexpr void Insert(InputIt first, InputIt last)
        {
            for (; first != last; ++first) Insert(*first);
        }
        constexpr void Insert(InitializerList<ValueType> ilist)
        {
            Insert(ilist.begin(), ilist.end());
        }
        constexpr InsertReturnValue Insert(NodeType&& nh)
        {
            if (!nh) return {end(), false, {}};

            auto it = m_Tree.Find(nh.Node->Key());
            if (it != end()) return {it, false, Move(nh)};

            auto pos = m_Tree.Insert(nh.Node);
            nh.Node  = nullptr;
            return {pos, true, {}};
        }
        constexpr Iterator Insert(ConstIterator, NodeType&& nh)
        {
            auto result = Insert(Move(nh));
            return result.Position;
        }
        template <typename K>
        constexpr KeyValuePair<Iterator, bool> Insert(K&& x)
        {
            ValueType value(Forward<K>(x));
            return Insert(Move(value));
        }
        template <typename K>
        constexpr Iterator Insert(ConstIterator, K&& x)
        {
            auto result = Insert(Forward<K>(x));
            return result.First;
        }

        template <typename... Args>
        constexpr KeyValuePair<Iterator, bool> Emplace(Args&&... args)
        {
            ValueType value(Forward<Args>(args)...);
            return Insert(value);
        }

        SizeType Erase(const KeyType& key) { return m_Tree.Erase(key) ? 1 : 0; }

        Iterator Erase(Iterator pos)
        {
            auto next = pos;
            ++next;
            m_Tree.Erase(static_cast<typename TreeType::Node*>(pos));
            return next;
        }

        NodeType Extract(const KeyType& key)
        {
            auto it = m_Tree.Find(key);
            if (it == m_Tree.end()) return {};

            auto* node = static_cast<typename TreeType::Node*>(it);
            m_Tree.Erase(node);
            return node_type(node);
        }

        void Merge(Set& other)
        {
            for (auto it = other.begin(); it != other.end();)
            {
                auto current = it++;
                auto nh      = other.Extract(*current);
                Insert(Move(nh));
            }
        }
        Iterator      Find(const KeyType& key) { return m_Tree.Find(key); }

        ConstIterator Find(const KeyType& key) const
        {
            return m_Tree.Find(key);
        }

        bool Contains(const KeyType& key) const { return m_Tree.Contains(key); }

        SizeType Count(const KeyType& key) const
        {
            return Contains(key) ? 1 : 0;
        }

        Iterator LowerBound(const KeyType& key)
        {
            return m_Tree.FindLargestNotAbove(key);
        }

        Iterator UpperBound(const KeyType& key)
        {
            auto it = LowerBound(key);
            if (it != end() && !m_Compare(key, *it)) ++it;
            return it;
        }

        Iterator        begin() { return Iterator(m_Tree.begin()); }
        ConstIterator   begin() const { return ConstIterator(m_Tree.begin()); }
        ConstIterator   cbegin() const { return ConstIterator(m_Tree.begin()); }

        Iterator        end() { return Iterator(m_Tree.end()); }
        ConstIterator   end() const { return ConstIterator(m_Tree.end()); }
        ConstIterator   cend() const { return ConstIterator(m_Tree.end()); }

        ReverseIterator rbegin() { return ReverseIterator(end()); }
        ConstReverseIterator rbegin() const
        {
            return ConstReverseIterator(end());
        }

        ReverseIterator      rend() { return ReverseIterator(begin()); }
        ConstReverseIterator rend() const
        {
            return ConstReverseIterator(begin());
        }

      private:
        TreeType m_Tree;
        Compare  m_Compare;
    };
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
PRISM_EXPORT using Prism::Set;
#endif
