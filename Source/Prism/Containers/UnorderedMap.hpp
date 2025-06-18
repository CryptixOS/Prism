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

            constexpr KeyValuePair() = default;
            constexpr KeyValuePair(const KeyType& key, const ValueType& value)
                : Key(key)
                , Value(value)
            {
            }
            constexpr KeyValuePair(const KeyType& key, ValueType&& value)
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
            constexpr Node(const KeyType& key, const ValueType& value)
                : Entry(key, value)
            {
            }
            constexpr Node(const KeyType& key, ValueType&& value)
                : Entry(key, Move(value))
            {
            }
        };

        using NodeType   = Node;
        using BucketType = Node::List;

        template <bool Const = false>
        struct Iterator
        {
            using EntryRefType
                = ConditionalType<Const, const KeyValuePair&, KeyValuePair&>;
            using EntryPointerType
                = ConditionalType<Const, KeyValuePair* const, KeyValuePair*>;

            constexpr Iterator(
                ConditionalType<Const, const UnorderedMap*, UnorderedMap*> map,
                const usize index)
                : m_Map(map)
                , m_BucketIndex(index)
            {
                if (m_BucketIndex >= m_Map->Capacity()) return;

                ListIt = m_Map->m_Buckets[m_BucketIndex].begin();
                AdvanceToValid();
            }
            constexpr Iterator(
                ConditionalType<Const, const UnorderedMap*, UnorderedMap*> map,
                usize index, typename BucketType::Iterator listIt)
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

            constexpr EntryRefType operator*() const { return ListIt->Entry; }
            constexpr EntryPointerType operator->() const
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
            ConditionalType<Const, const UnorderedMap*, UnorderedMap*> m_Map
                = nullptr;
            usize                         m_BucketIndex = 0;
            typename BucketType::Iterator ListIt;
        };
        using ConstIterator = Iterator<true>;

        constexpr UnorderedMap(usize initialCapacity = 8)
        {
            m_Buckets.Resize(initialCapacity);
        }
        constexpr ~UnorderedMap();

        constexpr bool  Empty() const PM_NOEXCEPT { return m_Size == 0; }
        constexpr usize Size() const PM_NOEXCEPT { return m_Size; }
        constexpr usize Capacity() const PM_NOEXCEPT
        {
            return m_Buckets.Size();
        }

        constexpr void       Clear() PM_NOEXCEPT;
        constexpr Iterator<> Insert(const K& key, const V& value);
        constexpr Iterator<> Insert(const K& key, V&& value);
        template <typename... Args>
        constexpr Iterator<>       Emplace(Args&&... args);
        constexpr Iterator<>       Erase(const K& key);

        constexpr ValueType&       At(const KeyType& key);
        constexpr const ValueType& At(const KeyType& key) const;
        constexpr ValueType&       operator[](const KeyType& key);
        constexpr ValueType&       operator[](KeyType&& key);

        constexpr Iterator<>       Find(const K& key);
        constexpr ConstIterator    Find(const K& key) const;
        constexpr bool             Contains(const K& key) const
        {
            return Find(key) != end();
        }

        constexpr void       Rehash(usize count);

        constexpr Iterator<> begin() PM_NOEXCEPT { return Iterator<>(this, 0); }
        constexpr ConstIterator begin() const PM_NOEXCEPT
        {
            return ConstIterator(this, 0);
        }
        constexpr ConstIterator cbegin() const PM_NOEXCEPT
        {
            return ConstIterator(this, 0);
        }

        constexpr Iterator<> end() PM_NOEXCEPT
        {
            return Iterator<>(this, Capacity());
        }
        constexpr ConstIterator end() const PM_NOEXCEPT
        {
            return ConstIterator(this, Capacity());
        }
        constexpr ConstIterator cend() const PM_NOEXCEPT
        {
            return ConstIterator(this, Capacity());
        }

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
