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
    void UnorderedMap<K, V, H>::Insert(const K& key, V&& value)
    {
        Rehash(m_Buckets.Size() * 2);
        usize index = HashType{}(key) % Size();
        for (auto& [k, v] : m_Buckets[index])
        {
            if (k == key)
            {
                v = value;
                return;
            }
        }

        m_Buckets[index].EmplaceBack(key, value);
    }
    template <typename K, typename V, typename H>
    usize UnorderedMap<K, V, H>::Erase(const K& key)
    {
        usize index           = HashType{}(key) % Size();
        auto& bucket          = m_Buckets[index];

        usize elementsRemoved = 0;
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
        {
            if (it->Key == key)
            {
                bucket.Erase(it);
                ++elementsRemoved;
            }
        }

        return elementsRemoved;
    }

    template <typename K, typename V, typename H>
    UnorderedMap<K, V, H>::Iterator UnorderedMap<K, V, H>::Find(const K& key)
    {
        for (auto& bucket : m_Buckets)
        {
            for (auto it = bucket.begin(); it != bucket.end(); it++)
            {
                if (it->Key == key) return it;
            }

            return m_Buckets.end();
        }
    }

    template <typename K, typename V, typename H>
    void UnorderedMap<K, V, H>::Rehash(usize count)
    {
        if (m_Buckets.Size() * m_LoadFactorDenominator
            < m_Buckets.Size() * m_LoadFactorNumerator)
            return;

        auto oldTable = std::move(m_Buckets);
        m_Buckets.Reserve(count);

        for (auto& bucket : oldTable)
        {
            for (auto& [key, value] : bucket)
            {
                usize newIndex = HashType{}(key) % count;
                m_Buckets[newIndex].EmplaceBack(std::move(key),
                                                std::move(value));
            }
        }
    }
}; // namespace Prism
