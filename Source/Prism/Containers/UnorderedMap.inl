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
    constexpr UnorderedMap<K, V, H>::~UnorderedMap()
    {
        Clear();
    }

    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::Clear() PM_NOEXCEPT
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
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Insert(const K& key, const V& value)
    {
        if (m_Size * m_LoadFactorDenominator
            >= Capacity() * m_LoadFactorNumerator)
            Rehash(Capacity() * 2);

        auto  index  = HashType{}(key) % Capacity();
        auto& bucket = m_Buckets[index];

        for (auto it = bucket.begin(); it != bucket.end(); it++)
        {
            auto& [k, v] = it->Entry;
            if (k == key)
            {
                v = value;
                return Iterator<>(this, index, it);
            }
        }

        auto node = new Node(key, value);
        bucket.PushBack(node);
        ++m_Size;

        typename BucketType::Iterator newElement(bucket.Tail());
        return Iterator<>(this, index, newElement);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Insert(const KeyType& key, V&& value)
    {
        if (m_Size * m_LoadFactorDenominator
            >= Capacity() * m_LoadFactorNumerator)
            Rehash(Capacity() * 2);

        auto  index  = HashType{}(key) % Capacity();
        auto& bucket = m_Buckets[index];
        for (auto it = bucket.begin(); it != bucket.end(); it++)
        {
            auto& [k, v] = it->Entry;
            if (k == key)
            {
                v = Move(value);
                return Iterator<>(this, index, it);
            }
        }

        auto node = new Node(key, Move(value));
        bucket.PushBack(node);
        ++m_Size;

        typename BucketType::Iterator newElement(bucket.Tail());
        return Iterator<>(this, index, newElement);
    }

    template <typename K, typename V, typename H>
    template <typename... Args>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Emplace(Args&&... args)
    {
        auto node = new Node(Forward<Args>(args)...);

        if (m_Size * 4 >= m_Buckets.Size() * 3)
            Rehash(Capacity() == 0 ? 8 : Capacity() * 2);

        usize index = HashType{}(node->Key) % Capacity();
        m_Buckets[index].PushFront(node);
        ++m_Size;

        return Iterator<>(this, index, m_Buckets[index].begin());
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Erase(const K& key)
    {
        auto  index  = HashType{}(key) % Capacity();
        auto& bucket = m_Buckets[index];

        for (auto it = bucket.begin(); it != bucket.end(); it++)
        {
            auto node = it.operator->();
            if (node->Entry.Key == key)
            {
                bucket.Erase(node);
                delete node;

                --m_Size;
                auto next = ++it;
                return next == bucket.end() ? Iterator<>(this, index)
                                            : Iterator<>(this, index, next);
            }
        }

        return end();
    }

    template <typename K, typename V, typename H>
    constexpr V& UnorderedMap<K, V, H>::At(const KeyType& key)
    {
        return Find(key)->Value;
    }
    template <typename K, typename V, typename H>
    constexpr const V& UnorderedMap<K, V, H>::At(const KeyType& key) const
    {
        return Find(key)->Value;
    }
    template <typename K, typename V, typename H>
    constexpr V& UnorderedMap<K, V, H>::operator[](const KeyType& key)
    {
        auto it = Find(key);
        if (it != end()) return it->Value;

        return Insert(key, V())->Value;
    }
    template <typename K, typename V, typename H>
    constexpr V& UnorderedMap<K, V, H>::operator[](KeyType&& k)
    {
        auto key = Move(k);

        auto it  = Find(key);
        if (it != end()) return it->Value;

        return Insert(key, {})->Value;
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Find(const K& key)
    {
        for (usize i = 0; auto& bucket : m_Buckets)
        {
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
                if (it->Entry.Key == key) return Iterator<>(this, i, it);
            ++i;
        }

        return end();
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstIterator
    UnorderedMap<K, V, H>::Find(const K& key) const
    {
        for (usize i = 0; auto& bucket : m_Buckets)
        {
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
                if (it->Entry.Key == key) return ConstIterator(this, i, it);
            ++i;
        }

        return end();
    }

    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::Rehash(usize count)
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
