/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/RedBlackTree.hpp> // NOLINT(misc-header-include-cycle)

namespace Prism
{

    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator& RedBlackTree<K, V>::Iterator::operator++()
    {
        if (!m_Node) return *this;
        m_Prev = m_Node;
        m_Node = static_cast<typename RedBlackTree::Node*>(RedBlackTree::GetSuccessor(m_Node));
        
        return *this;
    }
    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator& RedBlackTree<K, V>::Iterator::operator--()
    {
        if (!m_Prev) return *this;
        m_Node = m_Prev;
        m_Prev = static_cast<typename RedBlackTree::Node*>(RedBlackTree::GetPredecessor(m_Prev));
        
        return *this;
    }

    template <typename K, typename V>
    RedBlackTree<K, V>::~RedBlackTree() { Clear(); }   

    template <typename K, typename V>
    V& RedBlackTree<K, V>::At(const K& key)
    {
        Node* current = m_Root;
        while (m_Root && m_Root->Key != key)
            current = key < current->Key ? current->LeftChild : current->RightChild;

        return current;
    }
    template <typename K, typename V>
    const V&        RedBlackTree<K, V>::At(const K& key) const
    {
        return At(key);
    }

    template <typename K, typename V>
    V&              RedBlackTree<K, V>::operator[](const K& key)
    {
        return At(key);
    }
    template <typename K, typename V>
    V&              RedBlackTree<K, V>::operator[](K&& key)
    {
        return At(key);
    }


    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator                                    RedBlackTree<K, V>::begin()
    {
        return Node(m_Least);
    }
    template <typename K, typename V>
    const RedBlackTree<K, V>::Iterator                              RedBlackTree<K, V>::begin() const
    {
        return begin();
    }
    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator                                    RedBlackTree<K, V>::end()
    {
        return {};
    }
    template <typename K, typename V>
    const RedBlackTree<K, V>::Iterator                              RedBlackTree<K, V>::end() const
    {
        return {};
    }
    template <typename K, typename V>
    std::reverse_iterator<typename RedBlackTree<K, V>::Iterator>             RedBlackTree<K, V>::rbegin()
    {
        return {};
    }
    template <typename K, typename V>
    const std::reverse_iterator<const typename RedBlackTree<K, V>::Iterator> RedBlackTree<K, V>::rbegin() const
    {
        return {};
    }
    template <typename K, typename V>
    std::reverse_iterator<typename RedBlackTree<K, V>::Iterator>             RedBlackTree<K, V>::rend()
    {
        return begin();
    }
    template <typename K, typename V>
    const std::reverse_iterator<const typename RedBlackTree<K, V>::Iterator> RedBlackTree<K, V>::rend() const
    {
        return begin();
    }

    template <typename K, typename V>
    void RedBlackTree<K, V>::Clear()
    {
        RecursiveDelete(m_Root);

        m_Root = nullptr;
        m_Least = nullptr;
        m_Size = 0;
    }
    template <typename K, typename V>
    void RedBlackTree<K, V>::Insert(K& key, V& value)
    {
        auto* node = new Node(key, value);
        
        if (!m_Root)
        { 
            m_Root = node;
            m_Root->Color = Color::eBlack;
            m_Size = 1;
            m_Least = node;
            return;
        }
        
        Node* current = m_Root;
        while (current->LeftChild || current->RightChild)
        {
            if (Compare(key, current->Key) == -1) current = current->LeftChild;
            else current = current->RightChild;
        }

        node->Parent = current;
        if (Compare(key, current->Key) == -1) current->LeftChild = node;
        else current->RightChild = node;
        
        InsertFix(node);
        
        ++m_Size;
        if (m_Least->LeftChild == node)
            m_Least = node;
    }

    template <typename K, typename V>
    RedBlackTree<K, V>::Iterator        RedBlackTree<K, V>::Erase(const RedBlackTree<K, V>::Iterator pos)
    {
        Node* node = pos->m_Node;
        Node* ret = (pos++)->m_Node;
        
        RemoveNode(node);   
        
        node->RightChild = nullptr;
        node->LeftChild = nullptr;
        delete node;
        
        return ret;
    }
    template <typename K, typename V>
    bool RedBlackTree<K, V>::Erase(const K& key)
    {
        Node* node = Find(m_Root, key);
        if (!node) return false;

        RemoveNode(node);

        node->RightChild = nullptr;
        node->LeftChild = nullptr;
        delete node;
        
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
    void RedBlackTree<K, V>::PrintNode(const std::string& prefix, const Node* node, bool isLeft)
    {
        if (node)
        {
             PrismMessage("{}", prefix);

             PrismMessage("{}", isLeft ? "├──" : "└──" );

            // print the value of the node
             PrismMessage("{}{}\n", node->Key, node->Color == Color::eBlack ? "B" : "R", node->Color == Color::eBlack ? "B" : "R");

            // enter the next tree level - left and right branch
            PrintNode( prefix + (isLeft ? "│   " : "    "), node->LeftChild, true);
            PrintNode( prefix + (isLeft ? "│   " : "    "), node->RightChild, false);
        }
    }

    template <typename K, typename V>
    typename RedBlackTree<K, V>::Node* RedBlackTree<K, V>::Find(Node* node, K key)
    {
        while (node && node->Key == key)
            node = key < node->Key ? node->LeftChild : node->RightChild;

        return node;
    }


    template <typename K, typename V>
    typename RedBlackTree<K, V>::Node* RedBlackTree<K, V>::GetSuccessor(Node* node)
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
    typename RedBlackTree<K, V>::Node* RedBlackTree<K, V>::GetPredecessor(Node* node)
    {
        assert(node);
        if (node->LeftChild) 
        {
            node = node->LeftChild;
            while (node->RightChild)
                node = node->RightChild;
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
        assert(node);

        Node* current = node;
        while (current->Parent && current->Parent->Color == Color::eRed)
        {
            Node* grandParent = current->Parent->Parent;
            if (current->Parent == grandParent->LeftChild)
            {
                Node* uncle = grandParent->RightChild;
                if (uncle && uncle->Color == Color::eRed)
                {
                    grandParent->Color = Color::eRed;
                    grandParent->LeftChild->Color = Color::eBlack;
                    grandParent->RightChild->Color = Color::eBlack;
                    current = grandParent;
                }
                else 
                {
                    if (current == current->Parent->RightChild)
                    {
                        current = current->Parent;
                        RotateLeft(current);
                    }
                    current->Parent->Color = Color::eBlack;
                    grandParent->Color = Color::eRed;
                    RotateRight(grandParent);
                }
            }
            else
            {
                if (grandParent->LeftChild && grandParent->LeftChild->Color == Color::eRed)
                {
                    grandParent->LeftChild->Color = Color::eBlack;
                    grandParent->RightChild->Color = Color::eBlack;
                    grandParent->Color = Color::eRed;
                    current = grandParent;
                }
                else 
                {
                    if (current == current->Parent->LeftChild)
                    {
                        current = current->Parent;
                        RotateRight(current);
                    }
                    current->Parent->Color = Color::eBlack;
                    grandParent->Color = Color::eRed;
                    RotateLeft(grandParent);
                }
            }
        }

        m_Root->Color = Color::eBlack;
    }
    template <typename K, typename V>
    void  RedBlackTree<K, V>::RemoveFix(Node* node, Node* parent)
    {
        while (node != m_Root && (!node || node->Color == Color::eBlack)) 
        {
            if (parent->LeftChild == node) 
            {
                Node* sibling = parent->RightChild;
                if (sibling->Color == Color::eRed) 
                {
                    sibling->Color = Color::eBlack;
                    parent->Color = Color::eRed;
                    RotateLeft(parent);
                    sibling = parent->RightChild;
                }
                if ((!sibling->LeftChild || sibling->LeftChild->Color == Color::eBlack) && (!sibling->RightChild || sibling->RightChild->Color == Color::eBlack)) 
                {
                    sibling->Color = Color::eRed;
                    node = parent;
                } 
                else 
                {
                    if (!sibling->RightChild || sibling->RightChild->Color == Color::eBlack) 
                    {
                        sibling->LeftChild->Color = Color::eBlack; // null check?
                        sibling->Color = Color::eRed;
                        RotateRight(sibling);
                        sibling = parent->RightChild;
                    }
                    sibling->Color = parent->Color;
                    parent->Color = Color::eBlack;
                    sibling->RightChild->Color = Color::eBlack; // null check?
                    RotateLeft(parent);
                    node = m_Root; // fixed
                }
            } 
            else 
            {
                auto* sibling = parent->LeftChild;
                if (sibling->Color == Color::eRed) {
                    sibling->Color = Color::eBlack;
                    parent->Color = Color::eRed;
                    RotateRight(parent);
                    sibling = parent->LeftChild;
                }
                if ((!sibling->LeftChild || sibling->LeftChild->Color == Color::eBlack) && (!sibling->RightChild || sibling->RightChild->Color == Color::eBlack)) 
                {
                    sibling->Color = Color::eRed;
                    node = parent;
                } 
                else 
                {
                    if (!sibling->LeftChild || sibling->LeftChild->Color == Color::eBlack) 
                    {
                        sibling->RightChild->Color = Color::eBlack; // null check?
                        sibling->Color = Color::eRed;
                        RotateLeft(sibling);
                        sibling = parent->LeftChild;
                    }
                    sibling->Color = parent->Color;
                    parent->Color = Color::eBlack;
                    sibling->LeftChild->Color = Color::eBlack; // null check?
                    RotateRight(parent);
                    node = m_Root; // fixed
                }
            }
            parent = node->Parent;
        }
        node->color = Color::eBlack; // by this point node can't be null
    }


    template <typename K, typename V>
    i32 RedBlackTree<K, V>::Compare(const K& lhs, const K& rhs) const
    {
        if (lhs < rhs) return -1;
        
        return lhs == rhs;
    }

    template <typename K, typename V>
    void RedBlackTree<K, V>::RotateLeft(Node* node)
    {
        assert(node && node->RightChild);
        Node* rightChild = node->RightChild;

        node->RightChild = rightChild->LeftChild;
        if (rightChild->LeftChild) rightChild->LeftChild->Parent = node;

        rightChild->LeftChild = node;
        rightChild->Parent = node->Parent;
        node->Parent = rightChild;
        
        if (m_Root == node)
        {
            m_Root = rightChild;
            return;
        }
        
        if (rightChild->Parent->LeftChild == node)
            rightChild->Parent->LeftChild = rightChild;
        else rightChild->Parent->RightChild = rightChild;
    }
    template <typename K, typename V>
    void RedBlackTree<K, V>::RotateRight(Node* node)
    {
        assert(node->LeftChild);
	    auto temp = node->LeftChild;

  	    // update the two nodes
        node->LeftChild = temp->RightChild;
        if (temp->RightChild )
                temp->RightChild->Parent = node;
        temp->RightChild = node;
        temp->Parent = node->Parent;
        node->Parent = temp;

        // update the parent
        if (m_Root == node) 
        {
            m_Root = temp;
            return;
        }
        if (temp->Parent->LeftChild == node)
            temp->Parent->LeftChild = temp;
        else
      		temp->Parent->RightChild = temp;
    }
    template <typename K, typename V>
    void RedBlackTree<K, V>::RemoveNode(Node* node)
    {
        assert(node);
        if (m_Size == 1)
        {
            m_Root = nullptr;
            m_Least = nullptr;
            m_Size = 0;
            return;
        }

        if (m_Least == node) 
            m_Least = Successor(node);
        
        Color nodeToEraseColor = node->Color;   
        if (node->LeftChild && node->RightChild)
        {
            Node* successor = GetSuccessor(node);
            bool neighborSwap = successor->Parent == node;
            node->LeftChild->Parent = successor;

            if (!neighborSwap) node->RightChild->Parent = successor;
            if (node->Parent)
            {
                if (node->Parent->LeftChild == node) node->Parent->LeftChild = successor;
                else node->Parent->RightChild = successor;
            }
            else m_Root = successor;
            
            if (successor->RightChild) successor->RightChild->Parent = node;
            if (neighborSwap)
            {
                successor->Parent = node->Parent;
                node->Parent = successor;
            }
            else
            {
                if (successor->Parent)
                {
                    if (successor->Parent->LeftChild == successor)
                        successor->Parent->LeftChild = node;
                    else successor->Parent->RightChild = node;
                }
                else m_Root = node;
                std::swap(node->Parent, successor->Parent);
            }
            std::swap(node->LeftChild, successor->LeftChild);
            if (neighborSwap)
            {
                node->RightChild =  successor->RightChild;
                successor->RightChild = node;
            }
            else std::swap(node->RightChild, successor->RightChild);
            std::swap(node->Color, successor->Color);
        }

        Node* child = node->LeftChild ?: node->RightChild;

        if (child) child->Parent = node->Parent;
        if (node->Parent)
        {
            if (node->Parent->LeftChild == node) node->Parent->LeftChild = child;
            else node->Parent->RightChild = child;
        }
        else m_Root = child;

        if (node->Color != Color::eRed) RemoveFix(child, node->Parent);
        --m_Size;
    }

    template <typename K, typename V>
    void RedBlackTree<K, V>::RecursiveDelete(Node* node)
    {
        if (!node) return;
        RecursiveDelete(node->LeftChild);
        RecursiveDelete(node->RightChild);
        
        delete node;
    }
};
