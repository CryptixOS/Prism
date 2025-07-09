/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/NonMovable.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

#include <iterator>
#include <magic_enum/magic_enum.hpp>

namespace Prism
{
    template <typename K, typename V>
    class RedBlackTree : public NonCopyable<RedBlackTree<K, V>>,
                         public NonMovable<RedBlackTree<K, V>>
    {
      public:
        enum class Color : bool
        {
            eRed,
            eBlack,
        };

        struct KeyValuePair
        {
            K Key;
            V Value;
        };
        struct Node
        {
            Node*        Parent     = nullptr;
            Node*        LeftChild  = nullptr;
            Node*        RightChild = nullptr;

            enum Color   Color      = Color::eRed;
            KeyValuePair Data;

            Node() = default;
            Node(const K& key)
                : Data(key, V())
            {
            }
            Node(K&& key)
                : Data(Move(key), V())
            {
            }
            Node(const K& key, const V& value)
                : Data(key, value)
            {
            }
            Node(K&& key, V&& value)
                : Data(Move(key), Move(value))
            {
            }

            inline constexpr K&   Key() { return Data.Key; }
            inline constexpr V&   Value() { return Data.Value; }

            inline constexpr bool IsBlack() const
            {
                return Color == Color::eBlack;
            }
            inline constexpr bool IsRed() const { return Color == Color::eRed; }
            inline constexpr void SetBlack() { Color = Color::eBlack; }
            inline constexpr void SetRed() { Color = Color::eRed; }
        };

        template <typename ValueType>
        struct BaseIterator
        {
            BaseIterator() = default;
            constexpr bool operator!=(const BaseIterator& other) const
            {
                return m_Node != other.m_Node;
            }

            inline BaseIterator& operator++()
            {
                if (!m_Node) return *this;
                m_Prev = m_Node;
                m_Node = static_cast<typename RedBlackTree::Node*>(
                    RedBlackTree::GetSuccessor(m_Node));

                return *this;
            }
            inline BaseIterator& operator--()
            {
                if (!m_Prev) return *this;
                m_Node = m_Prev;
                m_Prev = static_cast<typename RedBlackTree::Node*>(
                    RedBlackTree::GetPredecessor(m_Prev));

                return *this;
            }

            inline KeyValuePair& operator*() { return m_Node->Data; }
            inline KeyValuePair* operator->() { return &m_Node->Data; }

            inline bool          IsFirst() const { return !m_Prev; }
            inline bool          IsLast() const { return !m_Node; }

            inline               operator Node*() const { return m_Node; }

          private:
            explicit BaseIterator(Node* node, Node* prev = nullptr)
                : m_Node(node)
                , m_Prev(prev)
            {
            }

            friend RedBlackTree;
            Node* m_Node = nullptr;
            Node* m_Prev = nullptr;
        };

        using Iterator      = BaseIterator<V>;
        using ConstIterator = BaseIterator<const V>;

        RedBlackTree()      = default;
        ~RedBlackTree();

        V&                                   At(const K& key);
        const V&                             At(const K& key) const;

        V&                                   operator[](const K& key);
        V&                                   operator[](K&& key);

        Iterator                             begin();
        ConstIterator                        begin() const;
        Iterator                             end();
        ConstIterator                        end() const;
        std::reverse_iterator<Iterator>      rbegin();
        std::reverse_iterator<ConstIterator> rbegin() const;
        std::reverse_iterator<Iterator>      rend();
        std::reverse_iterator<ConstIterator> rend() const;

        constexpr bool  IsEmpty() const { return m_Size == 0; }
        constexpr usize GetSize() const { return m_Size; }

        void            Clear();
        Iterator        Insert(K key, V& value);
        Iterator        Insert(Node* node);
        bool            Erase(const K& key);
        bool            Erase(Node* node);

        Iterator        Find(const K& key);
        ConstIterator   Find(const K& key) const;

        bool            Contains(const K& key) const;

        inline void     PrintTree();
        void         PrintNode(const fmt::string_view& prefix, const Node* node,
                               bool isLeft = false);

        static Node* Find(Node* node, K key);

        static Node* GetSuccessor(Node* node);
        static Node* GetPredecessor(Node* node);

      private:
        Node* m_Root      = nullptr;
        usize m_Size      = 0;

        Node* m_Least     = nullptr;
        Node* m_LeastNode = nullptr;

        void  InsertFix(Node* node);
        void  RemoveFix(Node* node, Node* parent);
        i32   Compare(const K& lhs, const K& rhs) const;

        void  RotateLeft(Node* node);
        void  RotateRight(Node* node);

        void  RecursiveDelete(Node* node);
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::RedBlackTree;
#endif

#include <Prism/Containers/RedBlackTree.inl>
