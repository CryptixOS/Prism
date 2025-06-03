/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/RedBlackTree.hpp> // NOLINT(misc-header-include-cycle)

namespace Prism
{

    template <typename K, typename V>
    RedBlackTree<K, V>::~RedBlackTree()
    {
        Clear();
    }

    template <typename K, typename V>
    V& RedBlackTree<K, V>::At(const K& key)
    {
        auto node = Find(m_Root, key);
        assert(node);

        return node->Value();
    }
    template <typename K, typename V>
    const V& RedBlackTree<K, V>::At(const K& key) const
    {
        auto node = Find(m_Root, key);
        assert(node);

        return node;
    }

    template <typename K, typename V>
    V& RedBlackTree<K, V>::operator[](const K& key)
    {
        Node* node = Find(key);
        if (node) return node->Value();

        return (*Insert(new Node(key))).Value;
    }
    template <typename K, typename V>
    V& RedBlackTree<K, V>::operator[](K&& key)
    {
        Node* node = Find(key);
        if (node) return node->Value();

        return (*Insert(new Node(Move(key)))).Value;
    }

    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator RedBlackTree<K, V>::begin()
    {
        return Iterator(m_LeastNode);
    }
    template <typename K, typename V>
    RedBlackTree<K, V>::ConstIterator RedBlackTree<K, V>::begin() const
    {
        return ConstIterator(m_LeastNode);
    }
    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator RedBlackTree<K, V>::end()
    {
        return {};
    }
    template <typename K, typename V>
    RedBlackTree<K, V>::ConstIterator RedBlackTree<K, V>::end() const
    {
        return {};
    }
    template <typename K, typename V>
    std::reverse_iterator<typename RedBlackTree<K, V>::Iterator>
    RedBlackTree<K, V>::rbegin()
    {
        return {};
    }
    template <typename K, typename V>
    std::reverse_iterator<typename RedBlackTree<K, V>::ConstIterator>
    RedBlackTree<K, V>::rbegin() const
    {
        return {};
    }
    template <typename K, typename V>
    std::reverse_iterator<typename RedBlackTree<K, V>::Iterator>
    RedBlackTree<K, V>::rend()
    {
        return Iterator(m_LeastNode);
    }
    template <typename K, typename V>
    std::reverse_iterator<typename RedBlackTree<K, V>::ConstIterator>
    RedBlackTree<K, V>::rend() const
    {
        return ConstIterator(m_LeastNode);
    }

    template <typename K, typename V>
    void RedBlackTree<K, V>::Clear()
    {
        RecursiveDelete(m_Root);

        m_Root  = nullptr;
        m_Least = nullptr;
        m_Size  = 0;
    }
    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator RedBlackTree<K, V>::Insert(K key, V& value)
    {
        auto* node = Find(m_Root, key);
        if (node)
        {
            node->Data.Value = value;
            return Iterator(node);
        }

        return Insert(new Node(key, value));
    }
    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator RedBlackTree<K, V>::Insert(Node* node)
    {
        Node* parent  = nullptr;
        Node* current = m_Root;
        while (current)
        {
            parent  = current;
            current = (node->Key() < current->Key()) ? current->LeftChild
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

        if (node->Key() < parent->Key()) parent->LeftChild = node;
        else parent->RightChild = node;
        node->Parent = parent;

        if (node->Parent->Parent) InsertFix(node);
        ++m_Size;

        if (m_LeastNode->LeftChild == node) m_LeastNode = node;
        return Iterator(node);
    }

    template <typename K, typename V>
    bool RedBlackTree<K, V>::Erase(const K& key)
    {
        Node* node = Find(m_Root, key);
        if (!node) return false;

        bool removed = Erase(node);

        delete node;
        return removed;
    }
    template <typename K, typename V>
    bool RedBlackTree<K, V>::Erase(Node* node)
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
            else
            {
                if (successorNode->Parent)
                {
                    if (successorNode->Parent->LeftChild == successorNode)
                        successorNode->Parent->LeftChild = node;
                    else successorNode->Parent->RightChild = node;
                }
                else m_Root = node;
                std::swap(node->Parent, successorNode->Parent);
            }
            std::swap(node->LeftChild, successorNode->LeftChild);
            if (neighborSwap)
            {
                node->RightChild          = successorNode->RightChild;
                successorNode->RightChild = node;
            }
            else std::swap(node->RightChild, successorNode->RightChild);
            std::swap(node->Color, successorNode->Color);
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

    template <typename K, typename V>
    void RedBlackTree<K, V>::PrintTree()
    {
        PrismInfo("RedBlackTree: Dump ->");
        PrismMessage("----------------\n");

        PrintNode("", m_Root, false);

        PrismMessage("\n");
        PrismMessage("----------------\n");
    }
    template <typename K, typename V>
    void RedBlackTree<K, V>::PrintNode(const fmt::string_view& prefix,
                                       const Node* node, bool isLeft)
    {
        if (node)
        {
            PrismMessage("{}", prefix);

            PrismMessage("{}", isLeft ? "├──" : "└──");

            // print the value of the node
            PrismMessage("{}{}\n", node->Key(),
                         node->Color == Color::eBlack ? "B" : "R",
                         node->Color == Color::eBlack ? "B" : "R");

            // enter the next tree level - left and right branch
            PrintNode(fmt::format("{}{}", prefix, isLeft ? "│   " : "    "),
                      node->LeftChild, true);
            PrintNode(fmt::format("{}{}", prefix, isLeft ? "│   " : "    "),
                      node->RightChild, false);
        }
    }

    template <typename K, typename V>
    typename RedBlackTree<K, V>::Iterator RedBlackTree<K, V>::Find(const K& key)
    {
        return Iterator(Find(m_Root, key));
    }
    template <typename K, typename V>
    typename RedBlackTree<K, V>::ConstIterator
    RedBlackTree<K, V>::Find(const K& key) const
    {
        return ConstIterator(Find(m_Root, key));
    }

    template <typename K, typename V>
    typename RedBlackTree<K, V>::Node* RedBlackTree<K, V>::Find(Node* node,
                                                                K     key)
    {
        while (node && node->Key() != key)
            node = key < node->Key() ? node->LeftChild : node->RightChild;

        return node;
    }

    template <typename K, typename V>
    bool RedBlackTree<K, V>::Contains(const K& key) const
    {
        Node* node = Find(m_Root, key);

        return node != nullptr;
    }

    template <typename K, typename V>
    typename RedBlackTree<K, V>::Node*
    RedBlackTree<K, V>::GetSuccessor(Node* node)
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
    template <typename K, typename V>
    typename RedBlackTree<K, V>::Node*
    RedBlackTree<K, V>::GetPredecessor(Node* node)
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

    template <typename K, typename V>
    void RedBlackTree<K, V>::InsertFix(Node* node)
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
    template <typename K, typename V>
    void RedBlackTree<K, V>::RemoveFix(Node* node, Node* parent)
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
                else
                {
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
            else
            {
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

    template <typename K, typename V>
    i32 RedBlackTree<K, V>::Compare(const K& lhs, const K& rhs) const
    {
        if (lhs < rhs) return -1;
        if (rhs < lhs) return 1;

        return 0;
    }

    template <typename K, typename V>
    void RedBlackTree<K, V>::RotateLeft(Node* node)
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
    template <typename K, typename V>
    void RedBlackTree<K, V>::RotateRight(Node* node)
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

    template <typename K, typename V>
    void RedBlackTree<K, V>::RecursiveDelete(Node* node)
    {
        if (!node) return;
        RecursiveDelete(node->LeftChild);
        RecursiveDelete(node->RightChild);

        delete node;
    }
}; // namespace Prism
