/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

export module Prism.Containers.RedBlackTree;

export import Prism.Utility.Compare;
export import Prism.Core.NonCopyable;
export import Prism.Core.NonMovable;
export import Prism.Debug.Log;
export import Prism.String.Formatter;

export extern "C++"
{
    namespace Prism
    {
        template <typename K, typename V, typename CompareOp = Less<K>>
        class RedBlackTree : public NonCopyable<RedBlackTree<K, V, CompareOp>>,
                             public NonMovable<RedBlackTree<K, V, CompareOp>>
        {
          public:
            using KeyType    = K;
            using ValueType  = V;
            using KeyCompare = CompareOp;

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
                using ReferenceType
                    = ConditionalType<IsConstV<ValueType>, const KeyValuePair&,
                                      KeyValuePair&>;
                using PointerType
                    = ConditionalType<IsConstV<ValueType>, const KeyValuePair*,
                                      KeyValuePair*>;

                BaseIterator() = default;

                template <typename U,
                          EnableIfType<IsSameV<ValueType, const U>, i32> = 0>
                BaseIterator(const BaseIterator<U>& other)
                    : m_Node(other.m_Node)
                    , m_Prev(other.m_Prev)
                {
                }

                constexpr bool operator==(const BaseIterator& other) const
                {
                    return m_Node == other.m_Node;
                }
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

                inline KeyValuePair& operator*() const { return m_Node->Data; }
                inline KeyValuePair* operator->() const { return &m_Node->Data; }

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

            using Iterator           = BaseIterator<V>;
            using ConstIterator      = BaseIterator<const V>;

            constexpr RedBlackTree() = default;
            constexpr RedBlackTree(const RedBlackTree& other);
            constexpr RedBlackTree(RedBlackTree&& other);
            explicit constexpr RedBlackTree(const CompareOp& comp)
                : m_Comparator(comp)
            {
            }

            constexpr RedBlackTree& operator=(const RedBlackTree& other);
            constexpr RedBlackTree& operator=(RedBlackTree&& other);

            constexpr ~RedBlackTree();

            V&                                    At(const K& key);
            const V&                              At(const K& key) const;

            V&                                    operator[](const K& key);
            V&                                    operator[](K&& key);

            Iterator                              begin();
            ConstIterator                         begin() const;
            Iterator                              end();
            ConstIterator                         end() const;
            Prism::ReverseIterator<Iterator>      rbegin();
            Prism::ReverseIterator<ConstIterator> rbegin() const;
            Prism::ReverseIterator<Iterator>      rend();
            Prism::ReverseIterator<ConstIterator> rend() const;

            constexpr bool  IsEmpty() const { return m_Size == 0; }
            constexpr usize GetSize() const { return m_Size; }

            void            Clear();
            Iterator        Insert(K key, V& value);
            Iterator        Insert(K key, V&& value);
            Iterator        Insert(Node* node);
            bool            Erase(const K& key);
            bool            Erase(Node* node);

            Iterator        FindLargestNotAbove(const K& key);
            Iterator        Find(const K& key);
            ConstIterator   Find(const K& key) const;
            Node*           Find(Node* node, const K& key, const CompareOp& comp);

            bool            Contains(const K& key) const;

            inline void     PrintTree();
            void         PrintNode(StringView prefix, const Node* node,
                                   bool isLeft = false);

            static Node* Find(Node* node, K key);

            static Node* GetSuccessor(Node* node);
            static Node* GetPredecessor(Node* node);

          private:
            CompareOp m_Comparator{};
            Node*     m_Root      = nullptr;
            usize     m_Size      = 0;

            Node*     m_Least     = nullptr;
            Node*     m_LeastNode = nullptr;

            void      InsertFix(Node* node);
            void      RemoveFix(Node* node, Node* parent);
            i32       Compare(const K& lhs, const K& rhs) const;
            i32       CompareKeys(const K& lhs, const K& rhs) const
            {
                if (m_Comparator(lhs, rhs)) return -1;
                if (m_Comparator(rhs, lhs)) return 1;
                return 0;
            }

            void  RotateLeft(Node* node);
            void  RotateRight(Node* node);

            void  RecursiveDelete(Node* node);

            Node* CloneSubtree(Node* parent, Node* other);
            Node* FindLeftMost(Node* node);
        };
    }; // namespace Prism
#if PRISM_USE_NAMESPACE != 0
    using Prism::RedBlackTree;
#endif


    namespace Prism
    {
        template <typename K, typename V, typename CompareOp>
        constexpr RedBlackTree<K, V, CompareOp>::RedBlackTree(
            const RedBlackTree& other)
        {
            if (!other.m_Root) return;

            m_Root      = CloneSubtree(nullptr, other.m_Root);
            m_Size      = other.m_Size;

            m_LeastNode = FindLeftmost(m_Root);
            m_Least     = m_LeastNode;
        }
        template <typename K, typename V, typename CompareOp>
        constexpr RedBlackTree<K, V, CompareOp>::RedBlackTree(RedBlackTree&& other)
        {
            m_Root            = other.m_Root;
            m_Size            = other.m_Size;
            m_Least           = other.m_Least;
            m_LeastNode       = other.m_LeastNode;

            other.m_Root      = nullptr;
            other.m_Size      = 0;
            other.m_Least     = nullptr;
            other.m_LeastNode = nullptr;
        }

        template <typename K, typename V, typename CompareOp>
        constexpr RedBlackTree<K, V, CompareOp>::~RedBlackTree()
        {
            Clear();
        }

        template <typename K, typename V, typename CompareOp>
        constexpr RedBlackTree<K, V, CompareOp>&
        RedBlackTree<K, V, CompareOp>::operator=(const RedBlackTree& other)
        {
            return (*this = other);
        }
        template <typename K, typename V, typename CompareOp>
        constexpr RedBlackTree<K, V, CompareOp>&
        RedBlackTree<K, V, CompareOp>::operator=(RedBlackTree&& other)
        {
            m_Root            = Move(other.m_Root);
            m_Size            = Move(other.m_Size);
            m_Least           = Move(other.m_Least);
            m_LeastNode       = Move(other.m_LeastNode);

            other.m_Root      = nullptr;
            other.m_Size      = 0;
            other.m_Least     = nullptr;
            other.m_LeastNode = nullptr;
            return *this;
        }

        template <typename K, typename V, typename CompareOp>
        V& RedBlackTree<K, V, CompareOp>::At(const K& key)
        {
            auto node = Find(m_Root, key);
            assert(node);

            return node->Value();
        }
        template <typename K, typename V, typename CompareOp>
        const V& RedBlackTree<K, V, CompareOp>::At(const K& key) const
        {
            auto node = Find(m_Root, key);
            assert(node);

            return node;
        }

        template <typename K, typename V, typename CompareOp>
        V& RedBlackTree<K, V, CompareOp>::operator[](const K& key)
        {
            Node* node = Find(key);
            if (node) return node->Value();

            return (*Insert(new Node(key))).Value;
        }
        template <typename K, typename V, typename CompareOp>
        V& RedBlackTree<K, V, CompareOp>::operator[](K&& key)
        {
            Node* node = Find(key);
            if (node) return node->Value();

            return (*Insert(new Node(Move(key)))).Value;
        }

        template <typename K, typename V, typename CompareOp>
        RedBlackTree<K, V, CompareOp>::Iterator
        RedBlackTree<K, V, CompareOp>::begin()
        {
            return Iterator(m_LeastNode);
        }
        template <typename K, typename V, typename CompareOp>
        RedBlackTree<K, V, CompareOp>::ConstIterator
        RedBlackTree<K, V, CompareOp>::begin() const
        {
            return ConstIterator(m_LeastNode);
        }
        template <typename K, typename V, typename CompareOp>
        RedBlackTree<K, V, CompareOp>::Iterator RedBlackTree<K, V, CompareOp>::end()
        {
            return {};
        }
        template <typename K, typename V, typename CompareOp>
        RedBlackTree<K, V, CompareOp>::ConstIterator
        RedBlackTree<K, V, CompareOp>::end() const
        {
            return {};
        }
        template <typename K, typename V, typename CompareOp>
        Prism::ReverseIterator<typename RedBlackTree<K, V, CompareOp>::Iterator>
        RedBlackTree<K, V, CompareOp>::rbegin()
        {
            return {};
        }
        template <typename K, typename V, typename CompareOp>
        Prism::ReverseIterator<
            typename RedBlackTree<K, V, CompareOp>::ConstIterator>
        RedBlackTree<K, V, CompareOp>::rbegin() const
        {
            return {};
        }
        template <typename K, typename V, typename CompareOp>
        Prism::ReverseIterator<typename RedBlackTree<K, V, CompareOp>::Iterator>
        RedBlackTree<K, V, CompareOp>::rend()
        {
            return Iterator(m_LeastNode);
        }
        template <typename K, typename V, typename CompareOp>
        Prism::ReverseIterator<
            typename RedBlackTree<K, V, CompareOp>::ConstIterator>
        RedBlackTree<K, V, CompareOp>::rend() const
        {
            return ConstIterator(m_LeastNode);
        }

        template <typename K, typename V, typename CompareOp>
        void RedBlackTree<K, V, CompareOp>::Clear()
        {
            RecursiveDelete(m_Root);

            m_Root  = nullptr;
            m_Least = nullptr;
            m_Size  = 0;
        }

        template <typename K, typename V, typename CompareOp>
        RedBlackTree<K, V, CompareOp>::Iterator
        RedBlackTree<K, V, CompareOp>::Insert(K key, V& value)
        {
            auto* node = Find(m_Root, key);
            if (node)
            {
                node->Data.Value = value;
                return Iterator(node);
            }

            return Insert(new Node(key, value));
        }
        template <typename K, typename V, typename CompareOp>
        RedBlackTree<K, V, CompareOp>::Iterator
        RedBlackTree<K, V, CompareOp>::Insert(K key, V&& value)
        {
            auto* node = Find(m_Root, key);
            if (node)
            {
                node->Data.Value = Move(value);
                return Iterator(node);
            }

            return Insert(new Node(key, Move(value)));
        }

        template <typename K, typename V, typename CompareOp>
        RedBlackTree<K, V, CompareOp>::Iterator
        RedBlackTree<K, V, CompareOp>::Insert(Node* node)
        {
            Node* parent  = nullptr;
            Node* current = m_Root;
            while (current)
            {
                parent  = current;
                current = m_Comparator(node->Key(), current->Key())
                            ? current->LeftChild
                            : current->RightChild;
            }

            if (!parent)
            {
                node->SetBlack();
                m_Root      = node;
                m_Size      = 1;
                m_LeastNode = node;
                return Iterator(m_Root);
            }

            if (m_Comparator(node->Key(), parent->Key())) parent->LeftChild = node;
            else parent->RightChild = node;
            node->Parent = parent;

            if (node->Parent->Parent) InsertFix(node);
            ++m_Size;

            if (m_LeastNode && m_Comparator(node->Key(), m_LeastNode->Key()))
                m_LeastNode = node;
            return Iterator(node);
        }

        template <typename K, typename V, typename CompareOp>
        bool RedBlackTree<K, V, CompareOp>::Erase(const K& key)
        {
            Node* node = Find(m_Root, key);
            if (!node) return false;

            bool removed = Erase(node);

            delete node;
            return removed;
        }
        template <typename K, typename V, typename CompareOp>
        bool RedBlackTree<K, V, CompareOp>::Erase(Node* node)
        {
            assert(node);

            if (m_Size == 1)
            {
                m_Root      = nullptr;
                m_LeastNode = nullptr;
                m_Size      = 0;
                return true;
            }

            if (m_LeastNode == node) m_LeastNode = GetSuccessor(node);
            if (node->LeftChild && node->RightChild)
            {
                auto* successorNode     = GetSuccessor(node);
                auto  neighborSwap      = successorNode->Parent == node;
                node->LeftChild->Parent = successorNode;
                if (!neighborSwap) node->RightChild->Parent = successorNode;
                if (node->Parent)
                {
                    if (node->Parent->LeftChild == node)
                        node->Parent->LeftChild = successorNode;
                    else node->Parent->RightChild = successorNode;
                }
                else m_Root = successorNode;
                if (successorNode->RightChild)
                    successorNode->RightChild->Parent = node;
                if (neighborSwap)
                {
                    successorNode->Parent = node->Parent;
                    node->Parent          = successorNode;
                }
                else {
                    if (successorNode->Parent)
                    {
                        if (successorNode->Parent->LeftChild == successorNode)
                            successorNode->Parent->LeftChild = node;
                        else successorNode->Parent->RightChild = node;
                    }
                    else m_Root = node;
                    Prism::Swap(node->Parent, successorNode->Parent);
                }
                Prism::Swap(node->LeftChild, successorNode->LeftChild);
                if (neighborSwap)
                {
                    node->RightChild          = successorNode->RightChild;
                    successorNode->RightChild = node;
                }
                else Prism::Swap(node->RightChild, successorNode->RightChild);
                Prism::Swap(node->Color, successorNode->Color);
            }

            auto* child = node->LeftChild ?: node->RightChild;

            if (child) child->Parent = node->Parent;
            if (node->Parent)
            {
                if (node->Parent->LeftChild == node)
                    node->Parent->LeftChild = child;
                else node->Parent->RightChild = child;
            }
            else m_Root = child;

            if (!node->IsRed()) RemoveFix(child, node->Parent);

            --m_Size;
            return true;
        }

        template <typename K, typename V, typename CompareOp>
        void RedBlackTree<K, V, CompareOp>::PrintTree()
        {
            PrismInfo("RedBlackTree: Dump ->");
            PrismMessage("----------------\n");

            PrintNode("", m_Root, false);

            PrismMessage("\n");
            PrismMessage("----------------\n");
        }
        template <typename K, typename V, typename CompareOp>
        void
        RedBlackTree<K, V, CompareOp>::PrintNode(StringView prefix,
                                                 const Node* node, bool isLeft)
        {
            if (node)
            {
                PrismMessage("{}", prefix);

                PrismMessage("{}", isLeft ? "├──" : "└──");

                // print the value of the node
                PrismMessage("{}{}\n", node->Key(),
                             node->Color == Color::eBlack ? "B" : "R");

                // enter the next tree level - left and right branch
                PrintNode(Format("{}{}", prefix, isLeft ? "│   " : "    "),
                          node->LeftChild, true);
                PrintNode(Format("{}{}", prefix, isLeft ? "│   " : "    "),
                          node->RightChild, false);
            }
        }

        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Iterator
        RedBlackTree<K, V, CompareOp>::FindLargestNotAbove(const K& key)
        {
            auto  node      = m_Root;
            Node* candidate = nullptr;
            while (node)
            {
                if (key == node->Key) return node;
                if (key < node->Key)
                {
                    node = node->LeftChild;
                    continue;
                }
                candidate = node;
                node      = node->RightChild;
            }

            return Iterator(candidate, key);
        }
        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Iterator
        RedBlackTree<K, V, CompareOp>::Find(const K& key)
        {
            return Iterator(Find(m_Root, key));
        }
        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::ConstIterator
        RedBlackTree<K, V, CompareOp>::Find(const K& key) const
        {
            return ConstIterator(Find(m_Root, key));
        }

        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Node*
        RedBlackTree<K, V, CompareOp>::Find(Node* node, K key)
        {
            while (node && node->Key() != key)
                node = key < node->Key() ? node->LeftChild : node->RightChild;

            return node;
        }
        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Node*
        RedBlackTree<K, V, CompareOp>::Find(Node* node, const K& key,
                                            const CompareOp& comp)
        {
            while (node)
            {
                if (comp(key, node->Key())) node = node->LeftChild;
                else if (comp(node->Key(), key)) node = node->RightChild;
                else return node;
            }
            return nullptr;
        }

        template <typename K, typename V, typename CompareOp>
        bool RedBlackTree<K, V, CompareOp>::Contains(const K& key) const
        {
            Node* node = Find(m_Root, key);

            return node != nullptr;
        }

        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Node*
        RedBlackTree<K, V, CompareOp>::GetSuccessor(Node* node)
        {
            assert(node);
            if (node->RightChild)
            {
                node = node->RightChild;
                while (node->LeftChild) node = node->LeftChild;
                return node;
            }
            auto temp = node->Parent;
            while (temp && node == temp->RightChild)
            {
                node = temp;
                temp = temp->Parent;
            }
            return temp;
        }
        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Node*
        RedBlackTree<K, V, CompareOp>::GetPredecessor(Node* node)
        {
            assert(node);
            if (node->LeftChild)
            {
                node = node->LeftChild;
                while (node->RightChild) node = node->RightChild;
                return node;
            }
            auto temp = node->Parent;
            while (temp && node == temp->LeftChild)
            {
                node = temp;
                temp = temp->Parent;
            }
            return temp;
        }

        template <typename K, typename V, typename CompareOp>
        void RedBlackTree<K, V, CompareOp>::InsertFix(Node* node)
        {
            assert(node && node->IsRed());

            while (node->Parent && node->Parent->Color == Color::eRed)
            {
                Node* grandParent = node->Parent->Parent;
                if (grandParent->RightChild == node->Parent)
                {
                    auto* uncle = grandParent->LeftChild;
                    if (uncle && uncle->IsRed())
                    {
                        node->Parent->SetBlack();
                        uncle->SetBlack();
                        grandParent->SetRed();
                        node = grandParent;
                        continue;
                    }

                    if (node->Parent->LeftChild == node)
                    {
                        node = node->Parent;
                        RotateRight(node);
                    }
                    node->Parent->SetBlack();
                    grandParent->SetRed();
                    RotateLeft(grandParent);
                    continue;
                }
                auto* uncle = grandParent->RightChild;
                if (uncle && uncle->IsRed())
                {
                    node->Parent->SetBlack();
                    uncle->SetBlack();
                    grandParent->SetRed();
                    node = grandParent;
                    continue;
                }

                if (node->Parent->RightChild == node)
                {
                    node = node->Parent;
                    RotateLeft(node);
                }
                node->Parent->SetBlack();
                grandParent->SetRed();
                RotateRight(grandParent);
            }

            m_Root->SetBlack();
        }
        template <typename K, typename V, typename CompareOp>
        void RedBlackTree<K, V, CompareOp>::RemoveFix(Node* node, Node* parent)
        {
            for (; node != m_Root && (!node || node->IsBlack());
                 parent = node->Parent)
            {
                if (parent->LeftChild == node)
                {
                    auto* sibling = parent->RightChild;
                    if (sibling->IsRed())
                    {
                        sibling->SetBlack();
                        parent->SetRed();
                        RotateLeft(parent);
                        sibling = parent->RightChild;
                    }
                    if ((!sibling->LeftChild || sibling->LeftChild->IsBlack())
                        && (!sibling->RightChild || sibling->RightChild->IsBlack()))
                    {
                        sibling->SetRed();
                        node = parent;
                    }
                    else {
                        if (!sibling->RightChild || sibling->RightChild->IsBlack())
                        {
                            sibling->LeftChild->SetBlack();
                            sibling->SetRed();
                            RotateRight(sibling);
                            sibling = parent->RightChild;
                        }
                        sibling->Color = parent->Color;
                        parent->SetBlack();
                        sibling->RightChild->SetBlack();
                        RotateLeft(parent);
                        node = m_Root;
                    }

                    continue;
                }
                auto* sibling = parent->LeftChild;
                if (sibling->IsRed())
                {
                    sibling->SetBlack();
                    parent->SetRed();
                    RotateRight(parent);
                    sibling = parent->LeftChild;
                }
                if ((!sibling->LeftChild || sibling->LeftChild->IsBlack())
                    && (!sibling->RightChild || sibling->RightChild->IsBlack()))
                {
                    sibling->SetRed();
                    node = parent;
                }
                else {
                    if (!sibling->LeftChild || sibling->LeftChild->IsBlack())
                    {
                        sibling->RightChild->SetBlack();
                        sibling->SetRed();
                        RotateLeft(sibling);
                        sibling = parent->LeftChild;
                    }
                    sibling->Color = parent->Color;
                    parent->SetBlack();
                    sibling->LeftChild->SetBlack();
                    RotateRight(parent);
                    node = m_Root;
                }
            }

            node->SetBlack();
        }

        template <typename K, typename V, typename CompareOp>
        i32 RedBlackTree<K, V, CompareOp>::Compare(const K& lhs, const K& rhs) const
        {
            if (lhs < rhs) return -1;
            if (rhs < lhs) return 1;

            return 0;
        }

        template <typename K, typename V, typename CompareOp>
        void RedBlackTree<K, V, CompareOp>::RotateLeft(Node* node)
        {
            assert(node);
            auto* pivot = node->RightChild;
            assert(pivot);
            auto* parent     = node->Parent;

            // stage 1 - node's right child is now pivot's left child
            node->RightChild = pivot->LeftChild;
            if (node->RightChild) node->RightChild->Parent = node;

            // stage 2 - pivot's left child is now node
            pivot->LeftChild = node;
            node->Parent     = pivot;

            // stage 3 - update pivot's parent
            pivot->Parent    = parent;
            if (!parent) m_Root = pivot;
            else if (parent->LeftChild == node) parent->LeftChild = pivot;
            else parent->RightChild = pivot;
        }
        template <typename K, typename V, typename CompareOp>
        void RedBlackTree<K, V, CompareOp>::RotateRight(Node* node)
        {
            assert(node);
            auto* pivot = node->LeftChild;
            assert(pivot);
            auto* parent    = node->Parent;

            // stage 1 - node's left child is now pivot's right child
            node->LeftChild = pivot->RightChild;
            if (node->LeftChild) node->LeftChild->Parent = node;

            // stage 2 - pivot's right child is now node
            pivot->RightChild = node;
            node->Parent      = pivot;

            // stage 3 - update pivot's parent
            pivot->Parent     = parent;
            if (!parent) m_Root = pivot;
            else if (parent->LeftChild == node) parent->LeftChild = pivot;
            else parent->RightChild = pivot;
        }

        template <typename K, typename V, typename CompareOp>
        void RedBlackTree<K, V, CompareOp>::RecursiveDelete(Node* node)
        {
            if (!node) return;
            RecursiveDelete(node->LeftChild);
            RecursiveDelete(node->RightChild);

            delete node;
        }

        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Node*
        RedBlackTree<K, V, CompareOp>::CloneSubtree(Node* parent, Node* other)
        {
            if (!other) return nullptr;

            Node* node       = new Node(other->Data.Key, other->Data.Value);
            node->Color      = other->Color;
            node->Parent     = parent;

            node->LeftChild  = CloneSubtree(node, other->LeftChild);
            node->RightChild = CloneSubtree(node, other->RightChild);

            return node;
        }
        template <typename K, typename V, typename CompareOp>
        typename RedBlackTree<K, V, CompareOp>::Node*
        RedBlackTree<K, V, CompareOp>::FindLeftMost(Node* node)
        {
            if (!node) return nullptr;
            while (node->LeftChild) node = node->LeftChild;
            return node;
        }
    }; // namespace Prism
}
