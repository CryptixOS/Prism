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
#include <utility>

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
        struct Node
        {
            Node* Parent     = nullptr;
            Node* LeftChild  = nullptr;
            Node* RightChild = nullptr;

            Color Color      = Color::eRed;
            K     Key;
            V     Value;

            Node() = default;
            Node(K& key, V& value)
                : Key(key)
                , Value(value)
            {
            }
            Node(K&& key, V&& value)
                : Key(std::move(key))
                , Value(std::move(value))
            {
            }
        };
        struct Iterator
        {
            Iterator() = default;
            constexpr bool operator!=(const Iterator& other) const
            {
                return m_Node != other.m_Node;
            }

            Iterator& operator++();
            Iterator& operator--();

            inline V& operator*() { return m_Node->Value; }
            inline V* operator->() { return &m_Node->Value; }

          private:
            explicit Iterator(Node* node, Node* prev = nullptr)
                : m_Node(node)
                , m_Prev(prev)
            {
            }

            friend RedBlackTree;
            Node* m_Node = nullptr;
            Node* m_Prev = nullptr;
        };

        RedBlackTree() = default;
        ~RedBlackTree();

        V&                                          At(const K& key);
        const V&                                    At(const K& key) const;

        V&                                          operator[](const K& key);
        V&                                          operator[](K&& key);

        Iterator                                    begin();
        const Iterator                              begin() const;
        Iterator                                    end();
        const Iterator                              end() const;
        std::reverse_iterator<Iterator>             rbegin();
        const std::reverse_iterator<const Iterator> rbegin() const;
        std::reverse_iterator<Iterator>             rend();
        const std::reverse_iterator<const Iterator> rend() const;

        constexpr bool  IsEmpty() const { return m_Size == 0; }
        constexpr usize GetSize() const { return m_Size; }

        void            Clear();
        void            Insert(K& key, V& value);
        // Iterator        Erase(Iterator pos);
        Iterator        Erase(const Iterator pos);
        Iterator        Erase(const Iterator first, const Iterator end);
        bool            Erase(const K& key);
        template <typename T>
        usize Erase(T&& x);
        bool  Find(const K& key, V& value) const;

        bool  Contains(const K& key) const;
        template <typename T>
        bool         Contains(const K& k) const;

        inline void  PrintTree();
        void         PrintNode(const std::string& prefix, const Node* node,
                               bool isLeft = false);

        static Node* Find(Node* node, K key);

        static Node* GetSuccessor(Node* node);
        static Node* GetPredecessor(Node* node);

      private:
        Node* m_Root  = nullptr;
        usize m_Size  = 0;

        Node* m_Least = nullptr;

        void  InsertFix(Node* node);
        void  RemoveFix(Node* node, Node* parent);
        i32   Compare(const K& lhs, const K& rhs) const;

        void  RotateLeft(Node* node);
        void  RotateRight(Node* node);
        void  RemoveNode(Node* node);

        void  RecursiveDelete(Node* node);
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::RedBlackTree;
#endif

#include <Prism/Containers/RedBlackTree.inl>
