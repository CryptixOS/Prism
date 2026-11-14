/*
 * Created by v1tr10l7 on 11.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

export module Prism.Containers.IntervalTree;

export import Prism.Containers.RedBlackTree;
export import Prism.Debug.Log;
export import Prism.Core.NonCopyable;
export import Prism.Core.NonMovable;
export import Prism.String.Formatter;

export extern "C++"
{
    namespace Prism
    {
        template <typename P>
        struct IntervalHook
        {
            P SubtreeMax;
        };

        template <typename T>
        struct Interval
        {
            T              Lower;
            T              Upper;

            constexpr bool Overlaps(T lb, T ub) const
            {
                return (Lower <= lb && lb <= Upper) || (lb <= Lower && Lower <= ub);
            }
        };

        /**
         * IntervalTree implementation using Prism::RedBlackTree.
         * K = Type for interval bounds (e.g., u64, usize)
         * V = Type for the payload data
         */
        // template <typename T, typename P, P T::* L, P T::* U,
        //           RedBlackTreeHook T ::* R, IntervalHook<P> T::* H>
        template <typename K, typename V>
        class IntervalTree : public RedBlackTree<K, V>
        {
          public:
            using Base = RedBlackTree<K, V>;
            using Node = typename Base::Node;

            /**
             * Augmented Node data to support interval logic.
             * We wrap the standard Node or extend it.
             */
            struct IntervalNode : public Node
            {
                K UpperBound;
                K SubtreeMax;

                IntervalNode(K lower, K upper, V value)
                    : Node(lower, Move(value))
                    , UpperBound(upper)
                    , SubtreeMax(upper)
                {
                }
            };

            IntervalTree() = default;
            ~IntervalTree()
            {
                // The base would delete these through Node*, which is not
                // an IntervalNode's real type.
                DeleteSubtree(static_cast<IntervalNode*>(this->m_Root));
                this->m_Root      = nullptr;
                this->m_LeastNode = nullptr;
                this->m_Size      = 0;
            }

            void InsertInterval(K lower, K upper, V value)
            {
                PrismCheck(lower <= upper);

                auto* newNode = new IntervalNode(lower, upper, Move(value));
                this->Insert(newNode);

                // Rebalancing rotates the new node's parent and grandparent,
                // which can leave one of them as a child of the path rather
                // than on it; refresh the path and its children.
                for (auto* node = newNode; node;
                     node       = static_cast<IntervalNode*>(node->Parent))
                {
                    if (node->LeftChild)
                        Refresh(static_cast<IntervalNode*>(node->LeftChild));
                    if (node->RightChild)
                        Refresh(static_cast<IntervalNode*>(node->RightChild));
                    Refresh(node);
                }
            }

            /**
             * Finds all intervals overlapping [lb, ub] and calls fn(Node*)
             */
            template <typename F>
            void ForOverlaps(K lb, K ub, F fn)
            {
                if (this->m_Root)
                {
                    SearchSubtree(static_cast<IntervalNode*>(this->m_Root), lb, ub,
                                  fn);
                }
            }

          private:
            static void Refresh(IntervalNode* node)
            {
                K max = node->UpperBound;
                if (node->LeftChild)
                    max = Max(max,
                              static_cast<IntervalNode*>(node->LeftChild)->SubtreeMax);
                if (node->RightChild)
                    max = Max(
                        max, static_cast<IntervalNode*>(node->RightChild)->SubtreeMax);
                node->SubtreeMax = max;
            }
            static void DeleteSubtree(IntervalNode* node)
            {
                if (!node) return;
                DeleteSubtree(static_cast<IntervalNode*>(node->LeftChild));
                DeleteSubtree(static_cast<IntervalNode*>(node->RightChild));
                delete node;
            }

            template <typename F>
            void SearchSubtree(IntervalNode* node, K lb, K ub, F& fn)
            {
                if (!node) return;

                // 1. If current node overlaps, execute callback
                if (Interval<K>{node->Key(), node->UpperBound}.Overlaps(lb, ub))
                {
                    fn(node);
                }

                // 2. If left child exists and its SubtreeMax >= lb, overlap might
                // be on the left
                if (node->LeftChild
                    && static_cast<IntervalNode*>(node->LeftChild)->SubtreeMax
                           >= lb)
                {
                    SearchSubtree(static_cast<IntervalNode*>(node->LeftChild), lb,
                                  ub, fn);
                }

                // 3. Check right child only if its potential intervals could start
                // before our ub AND the node's key (lower bound) is <= ub
                if (node->RightChild && node->Key() <= ub)
                {
                    SearchSubtree(static_cast<IntervalNode*>(node->RightChild), lb,
                                  ub, fn);
                }
            }
        };

    } // namespace Prism
}
