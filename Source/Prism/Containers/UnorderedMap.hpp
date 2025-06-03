/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/DoublyLinkedList.hpp>
#include <Prism/Containers/IntrusiveList.hpp>
#include <Prism/Containers/Vector.hpp>

#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/NonMovable.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Debug/Log.hpp>

namespace Prism
{
    template <typename K, typename V, typename H = std::hash<K>>
    class UnorderedMap
    {
      public:
        using KeyType   = K;
        using ValueType = V;
        using HashType  = H;

        struct KeyValuePair
        {
            KeyType   Key;
            ValueType Value;

            KeyValuePair() = default;
            KeyValuePair(const KeyType& key, ValueType&& value)
                : Key(key)
                , Value(Move(value))
            {
            }
        };

        struct Node
        {
            using KeyType   = K;
            using ValueType = V;

            KeyValuePair            Entry;

            IntrusiveListHook<Node> Hook;
            using List = IntrusiveList<Node>;

            constexpr Node()
                requires(IsDefaultConstructibleV<KeyType>
                         && IsDefaultConstructibleV<ValueType>)
            {
            }
            constexpr Node(const KeyType& key, ValueType&& value)
                : Entry(key, Move(value))
            {
            }
        };

        using NodeType   = Node;
        using BucketType = Node::List;

        struct Iterator
        {
            Iterator(UnorderedMap* map, usize index)
                : m_Map(map)
                , m_BucketIndex(index)
            {
                if (m_BucketIndex >= m_Map->Capacity()) return;

                ListIt = m_Map->m_Buckets[m_BucketIndex].begin();
                AdvanceToValid();
            }
            Iterator(UnorderedMap* map, usize index,
                     typename BucketType::Iterator listIt)
                : m_Map(map)
                , m_BucketIndex(index)
                , ListIt(listIt)
            {
            }

            constexpr inline void AdvanceToValid()
            {
                while (m_BucketIndex < m_Map->Capacity()
                       && ListIt == m_Map->m_Buckets[m_BucketIndex].end())
                {
                    ++m_BucketIndex;
                    if (m_BucketIndex < m_Map->Capacity())
                        ListIt = m_Map->m_Buckets[m_BucketIndex].begin();
                }
            }

            constexpr KeyValuePair& operator*() const { return ListIt->Entry; }
            constexpr KeyValuePair* operator->() const
            {
                return &ListIt->Entry;
            }

            constexpr Iterator& operator++()
            {
                ++ListIt;
                AdvanceToValid();

                return *this;
            }

            constexpr bool operator==(const Iterator& other) const
            {
                return m_BucketIndex == other.m_BucketIndex
                    && ListIt == other.ListIt;
            }
            constexpr bool operator!=(const Iterator& other) const
            {
                return m_BucketIndex != other.m_BucketIndex
                    || ListIt != other.ListIt;
            }

          private:
            UnorderedMap*                 m_Map         = nullptr;
            usize                         m_BucketIndex = 0;
            typename BucketType::Iterator ListIt;
        };

        UnorderedMap(usize initialCapacity = 8)
        {
            m_Buckets.Resize(initialCapacity);
        }
        ~UnorderedMap();

        bool     Empty() const PM_NOEXCEPT { return m_Size == 0; }
        usize    Size() const PM_NOEXCEPT { return m_Size; }
        usize    Capacity() const PM_NOEXCEPT { return m_Buckets.Size(); }

        // TODO(v1tr10l7): return iterator
        void     Insert(const K& key, const V& value);
        void     Insert(const K& key, V&& value);
        Iterator Erase(const K& key);

        Iterator Find(const K& key);
        bool     Contains(const K& key) const { return Find(key); }

        void     Rehash(usize count);

        constexpr Iterator begin() { return Iterator(this, 0); }
        constexpr Iterator end() { return Iterator(this, Capacity()); }

      private:
        Vector<BucketType> m_Buckets;
        usize              m_Size                  = 0;
        usize              m_LoadFactorNumerator   = 3;
        usize              m_LoadFactorDenominator = 4;
        usize              m_MaxLoadFactor         = 1;
    };
} // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::UnorderedMap;
#endif

#include <Prism/Containers/UnorderedMap.inl>
