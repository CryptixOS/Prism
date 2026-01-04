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

namespace Prism
{
    template <typename Key, typename Compare = Less<Key>>
    class Set : public NonCopyable<Set<Key, Compare>>,
                public NonMovable<Set<Key, Compare>>
    {
      public:
        // Member types
        using KeyType              = Key;
        using ValueType            = Key;
        using SizeType             = usize;
        using DifferenceType       = isize;
        using KeyCompare           = Compare;
        using ValueCompare         = Compare;

        using Reference            = ValueType&;
        using ConstReference       = const ValueType&;

        using Pointer              = ValueType*;
        using ConstPointer         = const ValueType*;

        using TreeType             = RedBlackTree<Key, Key>;

        using Iterator             = typename TreeType::Iterator;
        using ConstIterator        = typename TreeType::ConstIterator;
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

        Set()           = default;
        Set(const Set&) = default;

        explicit Set(const Compare& comp)
            : m_Compare(comp)
        {
        }

        bool     IsEmpty() const { return m_Tree.IsEmpty(); }
        SizeType GetSize() const { return m_Tree.GetSize(); }

        SizeType MaxSize() const { return NumericLimits<SizeType>::Max(); }

        void     Clear() { m_Tree.Clear(); }

        KeyValuePair<Iterator, bool> Insert(const ValueType& value)
        {
            auto it = m_Tree.Find(value);
            if (it != m_Tree.end()) return {it, false};

            return {m_Tree.Insert(value, const_cast<ValueType&>(value)), true};
        }
        template <typename... Args>
        KeyValuePair<Iterator, bool> Emplace(Args&&... args)
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
        InsertReturnValue Insert(NodeType&& nh)
        {
            if (!nh) return {end(), false, {}};

            auto it = m_Tree.Find(nh.Node->Key());
            if (it != end()) return {it, false, Move(nh)};

            auto pos = m_Tree.Insert(nh.Node);
            nh.Node  = nullptr;
            return {pos, true, {}};
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

        Iterator             begin() { return m_Tree.begin(); }
        ConstIterator        begin() const { return m_Tree.begin(); }
        ConstIterator        cbegin() const { return m_Tree.begin(); }

        Iterator             end() { return m_Tree.end(); }
        ConstIterator        end() const { return m_Tree.end(); }
        ConstIterator        cend() const { return m_Tree.end(); }

        ReverseIterator      rbegin() { return ReverseIterator(end()); }
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
