/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/UnorderedMap.hpp>

namespace Prism
{
    template <typename K, typename V, typename H>
    UnorderedMap<K, V, H>::~UnorderedMap()
    {
        for (auto& bucket : m_Buckets)
        {
            while (!bucket.Empty())
            {
                Node* node = bucket.Head();
                bucket.PopFront();
                node->Hook.Unlink(node);

                delete node;
                --m_Size;
            }
        }
    }

    template <typename K, typename V, typename H>
    void UnorderedMap<K, V, H>::Insert(const K& key, const V& value)
    {
        if (m_Size * m_LoadFactorDenominator
            >= Capacity() * m_LoadFactorNumerator)
            Rehash(Capacity() * 2);

        auto index = HashType{}(key) % Capacity();
        for (auto& it : m_Buckets[index])
        {
            auto& [k, v] = it.Entry;
            if (k == key)
            {
                v = value;
                return;
            }
        }

        auto node = new Node(key, value);
        m_Buckets[index].PushBack(node);
        ++m_Size;
    }
    template <typename K, typename V, typename H>
    void UnorderedMap<K, V, H>::Insert(const KeyType& key, V&& value)
    {
        if (m_Size * m_LoadFactorDenominator
            >= Capacity() * m_LoadFactorNumerator)
            Rehash(Capacity() * 2);

        auto index = HashType{}(key) % Capacity();
        for (auto& it : m_Buckets[index])
        {
            auto& [k, v] = it.Entry;
            if (k == key)
            {
                v = Move(value);
                return;
            }
        }

        auto node = new Node(key, Move(value));
        m_Buckets[index].PushBack(node);
        ++m_Size;
    }

    template <typename K, typename V, typename H>
    UnorderedMap<K, V, H>::Iterator UnorderedMap<K, V, H>::Erase(const K& key)
    {
        auto  index  = HashType{}(key) % Capacity();
        auto& bucket = m_Buckets[index];

        for (auto it = bucket.begin(); it != bucket.end(); it++)
        {
            auto* node = it.operator->();
            if (node->Entry.Key == key)
            {
                bucket.Erase(node);
                auto next = Iterator(this, index, ++it);
                delete node;

                --m_Size;
                return next;
            }
        }

        return end();
    }

    template <typename K, typename V, typename H>
    UnorderedMap<K, V, H>::Iterator UnorderedMap<K, V, H>::Find(const K& key)
    {
        for (usize i = 0; auto& bucket : m_Buckets)
        {
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
                if (it->Entry.Key == key) return Iterator(this, i, it);
            ++i;
        }

        return end();
    }

    template <typename K, typename V, typename H>
    void UnorderedMap<K, V, H>::Rehash(usize count)
    {
        if (Capacity() * m_LoadFactorDenominator
            < Capacity() * m_LoadFactorNumerator)
            return;

        auto oldTable = Move(m_Buckets);
        m_Buckets.Resize(count);
        m_Size = 0;

        for (auto& bucket : oldTable)
        {
            while (!bucket.Empty())
            {
                Node* node = bucket.Head();
                bucket.PopFront();
                node->Hook.Unlink(node);

                const K& key      = node->Entry.Key;
                auto     newIndex = HashType{}(key) % count;

                m_Buckets[newIndex].PushBack(node);
                ++m_Size;
            }
        }
    }
}; // namespace Prism
