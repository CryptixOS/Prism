/*
 * Created by v1tr10l7 on 11.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/RedBlackTree.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Memory/Ref.hpp>

namespace Prism
{
    namespace Details
    {
        template <typename T>
        struct _RawPtr
        {
            using Type = T*;
        };

        template <typename T, typename Container>
        struct SubstituteIntrusiveContainerType
        {
            using Type = Container;
        };
        template <typename T>
        struct SubstituteIntrusiveContainerType<T, Ref<T>>
        {
            using Type = Ref<T>;
        };
        template <typename Container, bool _IsRaw>
        struct SelfReferenceIfNeeded
        {
            Container Reference = nullptr;
        };
        template <typename Container>
        struct SelfReferenceIfNeeded<Container, true>
        {
        };

    } // namespace Details
    template <typename T>
    using RawPtr = typename Details::_RawPtr<T>::Type;

    template <Integral K>
    class BaseRedBlackTree : public NonCopyable<BaseRedBlackTree<K>>,
                             public NonMovable<BaseRedBlackTree<K>>
    {
      public:
        PM_NODISCARD usize Size() const { return m_Size; }
        PM_NODISCARD bool  IsEmpty() const { return m_Size == 0; }

        enum class Color : bool
        {
            Red,
            Black
        };
        struct Node
        {
            Node*      LeftChild{nullptr};
            Node*      RightChild{nullptr};
            Node*      Parent{nullptr};

            enum Color Color
            {
                Color::Red
            };

            K Key;

            Node(K key)
                : Key(key)
            {
            }
            Node() {}
            virtual ~Node() {};
        };

      protected:
        BaseRedBlackTree()          = default;
        // These are protected to ensure no one instantiates the
        // leaky base red black tree directly
        virtual ~BaseRedBlackTree() = default;

        void RotateLeft(Node* SubtreeRoot)
        {
            assert(SubtreeRoot);
            auto* pivot = SubtreeRoot->RightChild;
            assert(pivot);
            auto* Parent            = SubtreeRoot->Parent;

            // stage 1 - SubtreeRoot's right child is now pivot's left child
            SubtreeRoot->RightChild = pivot->LeftChild;
            if (SubtreeRoot->RightChild)
                SubtreeRoot->RightChild->Parent = SubtreeRoot;

            // stage 2 - pivot's left child is now SubtreeRoot
            pivot->LeftChild    = SubtreeRoot;
            SubtreeRoot->Parent = pivot;

            // stage 3 - update pivot's Parent
            pivot->Parent       = Parent;
            if (!Parent) m_Root = pivot;
            else if (Parent->LeftChild == SubtreeRoot)
                // we are the left child
                Parent->LeftChild = pivot;
            else // we are the right child
                Parent->RightChild = pivot;
        }

        void RotateRight(Node* SubtreeRoot)
        {
            assert(SubtreeRoot);
            auto* pivot = SubtreeRoot->LeftChild;
            assert(pivot);
            auto* Parent           = SubtreeRoot->Parent;

            // stage 1 - SubtreeRoot's left child is now pivot's right child
            SubtreeRoot->LeftChild = pivot->RightChild;
            if (SubtreeRoot->LeftChild)
                SubtreeRoot->LeftChild->Parent = SubtreeRoot;

            // stage 2 - pivot's right child is now SubtreeRoot
            pivot->RightChild   = SubtreeRoot;
            SubtreeRoot->Parent = pivot;

            // stage 3 - update pivot's Parent
            pivot->Parent       = Parent;
            if (!Parent)
                // new root
                m_Root = pivot;
            else if (Parent->LeftChild == SubtreeRoot)
                // we are the left child
                Parent->LeftChild = pivot;
            else // we are the right child
                Parent->RightChild = pivot;
        }

        static Node* Find(Node* node, K key)
        {
            while (node && node->Key != key)
                node = key < node->Key ? node->LeftChild : node->RightChild;
            return node;
        }

        static Node* FindLargestNotAbove(Node* node, K key)
        {
            Node* candidate = nullptr;
            while (node)
            {
                if (key == node->Key) return node;
                if (key < node->Key) node = node->LeftChild;
                else {
                    candidate = node;
                    node      = node->RightChild;
                }
            }
            return candidate;
        }

        static Node* FindSmallestNotBelow(Node* node, K key)
        {
            Node* candidate = nullptr;
            while (node)
            {
                if (node->Key == key) return node;

                if (node->Key <= key) node = node->RightChild;
                else {
                    candidate = node;
                    node      = node->LeftChild;
                }
            }
            return candidate;
        }

        void Insert(Node* node)
        {
            assert(node);
            Node* Parent = nullptr;
            Node* temp   = m_Root;
            while (temp)
            {
                Parent = temp;
                if (node->Key < temp->Key) temp = temp->LeftChild;
                else temp = temp->RightChild;
            }
            if (!Parent)
            {
                // new root
                node->Color = Color::Black;
                m_Root      = node;
                m_Size      = 1;
                m_Minimum   = node;
                return;
            }
            // we are the left child
            if (node->Key < Parent->key) Parent->LeftChild = node;
            // we are the right child
            else Parent->RightChild = node;
            node->Parent = Parent;

            // no fixups to be done for a height <= 2 tree
            if (node->Parent->Parent) InsertFixups(node);

            m_Size++;
            if (m_Minimum->LeftChild == node) m_Minimum = node;
        }

        void InsertFixups(Node* node)
        {
            assert(node && node->Color == Color::Red);
            while (node->Parent && node->Parent->Color == Color::Red)
            {
                auto* grandParent = node->Parent->Parent;
                if (grandParent->RightChild == node->Parent)
                {
                    auto* uncle = grandParent->LeftChild;
                    if (uncle && uncle->Color == Color::Red)
                    {
                        node->Parent->Color = Color::Black;
                        uncle->Color        = Color::Black;
                        grandParent->Color  = Color::Red;
                        node                = grandParent;
                    }
                    else {
                        if (node->Parent->LeftChild == node)
                        {
                            node = node->Parent;
                            RotateRight(node);
                        }
                        node->Parent->Color = Color::Black;
                        grandParent->Color  = Color::Red;
                        RotateLeft(grandParent);
                    }
                }
                else {
                    auto* uncle = grandParent->RightChild;
                    if (uncle && uncle->Color == Color::Red)
                    {
                        node->Parent->Color = Color::Black;
                        uncle->Color        = Color::Black;
                        grandParent->Color  = Color::Red;
                        node                = grandParent;
                    }
                    else {
                        if (node->Parent->RightChild == node)
                        {
                            node = node->Parent;
                            RotateLeft(node);
                        }
                        node->Parent->Color = Color::Black;
                        grandParent->Color  = Color::Red;
                        RotateRight(grandParent);
                    }
                }
            }
            // the root should always be black
            m_Root->Color = Color::Black;
        }

        void Remove(Node* node)
        {
            assert(node);

            // special case: deleting the only node
            if (m_Size == 1)
            {
                m_Root    = nullptr;
                m_Minimum = nullptr;
                m_Size    = 0;
                return;
            }

            if (m_Minimum == node) m_Minimum = Successor(node);

            // removal assumes the node has 0 or 1 child, so if we have 2,
            // relink with the successor first (by definition the successor has
            // no left child)
            // FIXME: since we dont know how a value is represented in the node,
            // we can't simply swap the values and keys, and instead we relink
            // the nodes
            //  in place, this is quite a bit more expensive, as well as much
            //  less readable, is there a better way?
            if (node->LeftChild && node->RightChild)
            {
                auto* successorNode = Successor(
                    node); // this is always non-null as all nodes besides the
                           // maximum node have a successor, and the maximum
                           // node has no right child
                auto neighborSwap       = successorNode->Parent == node;
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
                    Swap(node->Parent, successorNode->Parent);
                }
                Swap(node->LeftChild, successorNode->LeftChild);
                if (neighborSwap)
                {
                    node->RightChild          = successorNode->RightChild;
                    successorNode->RightChild = node;
                }
                else Swap(node->RightChild, successorNode->RightChild);
                Swap(node->Color, successorNode->Color);
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

            // if the node is red then child must be black, and just replacing
            // the node with its child should result in a valid tree (no change
            // to black height)
            if (node->Color != Color::Red) RemoveFixups(child, node->Parent);

            m_Size--;
        }

        // We maintain Parent as a separate argument since node might be null
        void RemoveFixups(Node* node, Node* Parent)
        {
            while (node != m_Root && (!node || node->Color == Color::Black))
            {
                if (Parent->LeftChild == node)
                {
                    auto* sibling = Parent->RightChild;
                    if (sibling->Color == Color::Red)
                    {
                        sibling->Color = Color::Black;
                        Parent->Color  = Color::Red;
                        RotateLeft(Parent);
                        sibling = Parent->RightChild;
                    }
                    if ((!sibling->LeftChild
                         || sibling->LeftChild->Color == Color::Black)
                        && (!sibling->RightChild
                            || sibling->RightChild->Color == Color::Black))
                    {
                        sibling->Color = Color::Red;
                        node           = Parent;
                    }
                    else {
                        if (!sibling->RightChild
                            || sibling->RightChild->Color == Color::Black)
                        {
                            // null check?
                            sibling->LeftChild->Color = Color::Black;
                            sibling->Color            = Color::Red;
                            RotateRight(sibling);
                            sibling = Parent->RightChild;
                        }
                        sibling->Color = Parent->Color;
                        Parent->Color  = Color::Black;
                        sibling->RightChild->Color
                            = Color::Black; // null check?
                        RotateLeft(Parent);
                        node = m_Root; // fixed
                    }
                }
                else {
                    auto* sibling = Parent->LeftChild;
                    if (sibling->Color == Color::Red)
                    {
                        sibling->Color = Color::Black;
                        Parent->Color  = Color::Red;
                        RotateRight(Parent);
                        sibling = Parent->LeftChild;
                    }
                    if ((!sibling->LeftChild
                         || sibling->LeftChild->Color == Color::Black)
                        && (!sibling->RightChild
                            || sibling->RightChild->Color == Color::Black))
                    {
                        sibling->Color = Color::Red;
                        node           = Parent;
                    }
                    else {
                        if (!sibling->LeftChild
                            || sibling->LeftChild->Color == Color::Black)
                        {
                            sibling->RightChild->Color
                                = Color::Black; // null check?
                            sibling->Color = Color::Red;
                            RotateLeft(sibling);
                            sibling = Parent->LeftChild;
                        }
                        sibling->Color            = Parent->Color;
                        Parent->Color             = Color::Black;
                        sibling->LeftChild->Color = Color::Black; // null check?
                        RotateRight(Parent);
                        node = m_Root; // fixed
                    }
                }
                Parent = node->Parent;
            }
            node->Color = Color::Black; // by this point node can't be null
        }

        static Node* Successor(Node* node)
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

        static Node* Predecessor(Node* node)
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

        Node* m_Root{nullptr};
        usize m_Size{0};
        // maintained for O(1) begin()
        Node* m_Minimum{nullptr};
    };

    namespace Details
    {
        template <Integral K, typename V, typename Container = RawPtr<V>>
        class IntrusiveRedBlackTreeNode;

        struct ExtractIntrusiveRedBlackTreeTypes
        {
            template <typename K, typename V, typename Container, typename T>
            static K Key(IntrusiveRedBlackTreeNode<K, V, Container> T::* x);
            template <typename K, typename V, typename Container, typename T>
            static V Value(IntrusiveRedBlackTreeNode<K, V, Container> T::* x);
            template <typename K, typename V, typename Container, typename T>
            static Container
            GetContainer(IntrusiveRedBlackTreeNode<K, V, Container> T::* x);
        };

        template <Integral K, typename V, typename Container = RawPtr<V>>
        using SubstitutedIntrusiveRedBlackTreeNode = IntrusiveRedBlackTreeNode<
            K, V,
            typename SubstituteIntrusiveContainerType<V, Container>::Type>;

        template <
            Integral K, typename V, typename Container,
            SubstitutedIntrusiveRedBlackTreeNode<K, V, Container> V::* Member>
        class IntrusiveRedBlackTree : public BaseRedBlackTree<K>
        {

          public:
            IntrusiveRedBlackTree() = default;
            virtual ~IntrusiveRedBlackTree() override { Clear(); }

            using BaseTree = BaseRedBlackTree<K>;
            using TreeNode
                = SubstitutedIntrusiveRedBlackTreeNode<K, V, Container>;

            Container Find(K key)
            {
                auto* node
                    = static_cast<TreeNode*>(BaseTree::Find(this->m_Root, key));
                if (!node) return nullptr;
                return NodeToValue(*node);
            }

            Container FindLargestNotAbove(K key)
            {
                auto* node = static_cast<TreeNode*>(
                    BaseTree::FindLargestNotAbove(this->m_Root, key));
                if (!node) return nullptr;
                return NodeToValue(*node);
            }

            Container FindSmallestNotBelow(K key)
            {
                auto* node = static_cast<TreeNode*>(
                    BaseTree::FindSmallestNotBelow(this->m_Root, key));
                if (!node) return nullptr;
                return NodeToValue(*node);
            }

            void Insert(K key, V& value)
            {
                auto& node = value.*Member;
                assert(!node.m_In_tree);
                static_cast<typename BaseTree::Node&>(node).key = key;
                BaseTree::Insert(&node);
                // Note: Self-reference ensures that the
                // object will keep a ref to itself when the
                // Container is a smart pointer.
                if constexpr (!TreeNode::IsRaw) node.m_Self.reference = &value;
                node.m_In_tree = true;
            }

            template <typename ElementType>
            class BaseIterator
            {
              public:
                BaseIterator() = default;
                bool operator!=(BaseIterator const& other) const
                {
                    return m_Node != other.m_Node;
                }
                BaseIterator& operator++()
                {
                    if (!m_Node) return *this;
                    m_Prev = m_Node;
                    // the complexity is O(logn) for each successor call, but
                    // the total complexity for all elements comes out to O(n),
                    // meaning the amortized cost for a single call is O(1)
                    m_Node
                        = static_cast<TreeNode*>(BaseTree::Successor(m_Node));
                    return *this;
                }
                BaseIterator& operator--()
                {
                    if (!m_Prev) return *this;
                    m_Node = m_Prev;
                    m_Prev
                        = static_cast<TreeNode*>(BaseTree::Predecessor(m_Prev));
                    return *this;
                }
                ElementType& operator*()
                {
                    assert(m_Node);
                    return *NodeToValue(*m_Node);
                }
                auto operator->()
                {
                    assert(m_Node);
                    return NodeToValue(*m_Node);
                }
                PM_NODISCARD bool IsEnd() const { return !m_Node; }
                PM_NODISCARD bool IsBegin() const { return !m_Prev; }
                PM_NODISCARD auto Key() const { return m_Node->key; }

              private:
                friend class IntrusiveRedBlackTree;
                explicit BaseIterator(TreeNode* node, TreeNode* prev = nullptr)
                    : m_Node(node)
                    , m_Prev(prev)
                {
                }
                TreeNode* m_Node{nullptr};
                TreeNode* m_Prev{nullptr};
            };

            using Iterator = BaseIterator<V>;
            Iterator begin()
            {
                return Iterator(static_cast<TreeNode*>(this->m_Minimum));
            }
            Iterator end() { return {}; }
            Iterator BeginFrom(K key)
            {
                return Iterator(
                    static_cast<TreeNode*>(BaseTree::Find(this->m_Root, key)));
            }
            Iterator BeginFrom(V& value) { return Iterator(&(value.*Member)); }

            using ConstIterator = BaseIterator<V const>;
            ConstIterator begin() const
            {
                return ConstIterator(static_cast<TreeNode*>(this->m_Minimum));
            }
            ConstIterator end() const { return {}; }
            ConstIterator BeginFrom(K key) const
            {
                return ConstIterator(
                    static_cast<TreeNode*>(BaseTree::Find(this->m_RootF, key)));
            }
            ConstIterator BeginFrom(V const& value) const
            {
                return Iterator(&(value.*Member));
            }

            bool Remove(K key)
            {
                auto* node
                    = static_cast<TreeNode*>(BaseTree::Find(this->m_Root, key));
                if (!node) return false;

                BaseTree::Remove(node);

                node->RightChild = nullptr;
                node->LeftChild  = nullptr;
                node->m_In_tree  = false;
                if constexpr (!TreeNode::IsRaw)
                    node->m_Self.reference = nullptr;

                return true;
            }

            void Clear()
            {
                ClearNodes(static_cast<TreeNode*>(this->m_Root));
                this->m_Root    = nullptr;
                this->m_Minimum = nullptr;
                this->m_Size    = 0;
            }

          private:
            static void ClearNodes(TreeNode* node)
            {
                if (!node) return;
                ClearNodes(static_cast<TreeNode*>(node->RightChild));
                node->RightChild = nullptr;
                ClearNodes(static_cast<TreeNode*>(node->LeftChild));
                node->LeftChild = nullptr;
                node->m_In_tree = false;
                if constexpr (!TreeNode::IsRaw)
                    node->m_Self.reference = nullptr;
            }

            static V* NodeToValue(TreeNode& node)
            {
                return BitCast<V*>(BitCast<u8*>(&node) - BitCast<u8*>(Member));
            }
        };

        template <Integral K, typename V, typename Container>
        class IntrusiveRedBlackTreeNode : public BaseRedBlackTree<K>::Node
        {
          public:
            ~IntrusiveRedBlackTreeNode() { assert(!IsInTree()); }

            PM_NODISCARD bool IsInTree() const { return m_In_tree; }

            PM_NODISCARD K    Key() const
            {
                return BaseRedBlackTree<K>::Node::Key;
            }

            static constexpr bool IsRaw = IsPointerV<Container>;

#if !defined(AK_COMPILER_CLANG)
          private:
            template <Integral TK, typename TV, typename TContainer,
                      SubstitutedIntrusiveRedBlackTreeNode<TK, TV, TContainer>
                          TV::* Member>
            friend class ::Prism::Details::IntrusiveRedBlackTree;
#endif

            bool                 m_In_tree{false};
            PM_NO_UNIQUE_ADDRESS SelfReferenceIfNeeded<Container, IsRaw> m_Self;
        };

        // Specialise IntrusiveRedBlackTree for Ref
        // By default, red black trees cannot contain null entries anyway, so
        // switch to Ref and just make the user-facing functions deref the
        // pointers.
        /*
                template <
                    Integral                                           K,
           typename V, SubstitutedIntrusiveRedBlackTreeNode<K, V, Ref<V>> V::*
           Member> class IntrusiveRedBlackTree<K, V, Ref<V>, Member> : public
           IntrusiveRedBlackTree<K, V, Ref<V>, Member>
                {
                  public:
                    PM_NODISCARD Ref<V> Find(K key) const
                    {
                        return IntrusiveRedBlackTree<K, V, Ref<V>,
           Member>::Find(key) .Raw();
                    }
                    PM_NODISCARD Ref<V> FindLargestNotAbove(K key) const
                    {
                        return IntrusiveRedBlackTree<K, V, Ref<V>,
                                                     Member>::FindLargestNotAbove(key)
                            .Release();
                    }
                    PM_NODISCARD Ref<V> FindSmallestNotBelow(K key) const
                    {
                        return IntrusiveRedBlackTree<K, V, Ref<V>,
                                                     Member>::FindSmallestNotBelow(key)
                            .Release();
                    }
                };
        */
    }; // namespace Details

    template <Integral K, typename V, typename Container = RawPtr<K>>
    using IntrusiveRedBlackTreeNode
        = Details::SubstitutedIntrusiveRedBlackTreeNode<K, V, Container>;

    template <auto Member>
    using IntrusiveRedBlackTree = Details::IntrusiveRedBlackTree<
        decltype(Details::ExtractIntrusiveRedBlackTreeTypes::Key(Member)),
        decltype(Details::ExtractIntrusiveRedBlackTreeTypes::Value(Member)),
        decltype(Details::ExtractIntrusiveRedBlackTreeTypes::GetContainer(
            Member)),
        Member>;
}; // namespace Prism
