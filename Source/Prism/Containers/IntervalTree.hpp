/*
 * Created by v1tr10l7 on 11.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/RedBlackTree.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

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

        void InsertInterval(K lower, K upper, V value)
        {
            assert(lower <= upper
                         && "Lower bound must be <= upper bound");

            auto* newNode = new IntervalNode(lower, upper, Move(value));
            this->Insert(newNode);

            // After standard RB-Insert, we must update SubtreeMax up to the
            // root
            UpdateSubtreeMax(newNode);
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
        void UpdateSubtreeMax(IntervalNode* node)
        {
            while (node)
            {
                K newMax = node->UpperBound;

                if (node->LeftChild)
                {
                    newMax = Max(newMax,
                                 static_cast<IntervalNode*>(node->LeftChild)
                                     ->SubtreeMax);
                }
                if (node->RightChild)
                {
                    newMax = Max(newMax,
                                 static_cast<IntervalNode*>(node->RightChild)
                                     ->SubtreeMax);
                }

                if (node->SubtreeMax == newMax)
                    break; // Optimization: stop if no change

                node->SubtreeMax = newMax;
                node             = static_cast<IntervalNode*>(node->Parent);
            }
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
