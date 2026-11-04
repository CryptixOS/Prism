/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/UnorderedMap.hpp>

PRISM_EXPORT namespace Prism
{
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::UnorderedMap(usize initialCapacity)
    {
        m_Buckets.Resize(initialCapacity);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::UnorderedMap(const UnorderedMap& other)
    {
        m_Buckets.Resize(other.Capacity() ? other.Capacity() : 8);
        for (const auto& entry : other) InsertOrAssign(entry.Key, entry.Value);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::UnorderedMap(
        UnorderedMap&& other) PM_NOEXCEPT
        : m_Buckets(Move(other.m_Buckets)),
          m_Size(Exchange(other.m_Size, 0))
    {
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>&
    UnorderedMap<K, V, H>::operator=(const UnorderedMap& other)
    {
        if (this == &other) return *this;

        UnorderedMap copy(other);
        return *this = Move(copy);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>&
    UnorderedMap<K, V, H>::operator=(UnorderedMap&& other) PM_NOEXCEPT
    {
        if (this == &other) return *this;

        Clear();
        m_Buckets = Move(other.m_Buckets);
        m_Size    = Exchange(other.m_Size, 0);

        return *this;
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::~UnorderedMap()
    {
        Clear();
    }

    template <typename K, typename V, typename H>
    constexpr V& UnorderedMap<K, V, H>::At(const KeyType& key)
    {
        auto it = Find(key);
        assert(it != end() && "UnorderedMap::At(): key not found");
        return it->Value;
    }
    template <typename K, typename V, typename H>
    constexpr const V& UnorderedMap<K, V, H>::At(const KeyType& key) const
    {
        auto it = Find(key);
        assert(it != end() && "UnorderedMap::At(): key not found");
        return it->Value;
    }
    template <typename K, typename V, typename H>
    constexpr V& UnorderedMap<K, V, H>::operator[](const KeyType& key)
    {
        auto it = Find(key);
        if (it != end()) return it->Value;

        it = InsertOrAssign(key, V());
        return it->Value;
    }
    template <typename K, typename V, typename H>
    constexpr const V&
    UnorderedMap<K, V, H>::operator[](const KeyType& key) const
    {
        return At(key);
    }
    template <typename K, typename V, typename H>
    constexpr V& UnorderedMap<K, V, H>::operator[](KeyType&& k)
    {
        auto key = Move(k);

        auto it  = Find(key);
        if (it != end()) return it->Value;

        return InsertOrAssign(Move(key), V())->Value;
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<> UnorderedMap<K, V, H>::begin()
    {
        return Iterator<>(this, false);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstIterator
    UnorderedMap<K, V, H>::begin() const
    {
        return ConstIterator(this, false);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstIterator
    UnorderedMap<K, V, H>::cbegin() const PM_NOEXCEPT
    {
        return ConstIterator(this, false);
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<> UnorderedMap<K, V, H>::end()
    {
        return Iterator<>(this, true);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstIterator
    UnorderedMap<K, V, H>::end() const
    {
        return ConstIterator(this, true);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstIterator
    UnorderedMap<K, V, H>::cend() const PM_NOEXCEPT
    {
        return ConstIterator(this, true);
    }

    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::Clear() PM_NOEXCEPT
    {
        // Keep the bucket array: begin()/Find()/Insert() rely on it.
        for (auto& bucket : m_Buckets)
            while (Node* node = bucket.PopFrontElement()) delete node;

        m_Size = 0;
    }
    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::Rehash(usize count)
    {
        if (count == 0) count = 1;
        if (count == Capacity()) return;

        auto oldTable = Move(m_Buckets);
        m_Buckets.Resize(count);

        for (auto& bucket : oldTable)
        {
            while (Node* node = bucket.PopFrontElement())
            {
                auto newIndex = HashType{}(node->Entry.Key) % count;
                m_Buckets[newIndex].PushBack(node);
            }
        }
    }

    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::Reserve(usize count)
    {
        if (Capacity() >= count) return;
        Rehash(count);
    }
    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::EnsureCapacity()
    {
        if (m_Size * 4 >= m_Buckets.Size() * 3)
            Rehash(Capacity() == 0 ? 8 : Capacity() * 2);
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::InsertOrAssign(const K& key, const V& value)
    {
        auto it = Find(key);
        if (it != end())
        {
            it->Value = value;
            return it;
        }

        return Insert(new Node({key, value}));
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::InsertOrAssign(const KeyType& key, V&& value)
    {
        auto it = Find(key);
        if (it != end())
        {
            it->Value = Move(value);
            return it;
        }

        return Insert(new Node({key, Move(value)}));
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::InsertOrAssign(K&& k, V&& value)
    {
        auto key = Move(k);

        auto it  = Find(key);
        if (it != end())
        {
            it->Value = Move(value);
            return it;
        }

        return Insert(new Node(Move(key), Move(value)));
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Insert(const KeyValuePair& entry)
    {
        auto it = Find(entry.Key);
        if (it != end()) return it;

        return Insert(new Node(entry.Key, entry.Value));
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Insert(KeyValuePair&& entry)
    {
        auto it = Find(entry.Key);
        if (it != end()) return it;

        return Insert(new Node(Move(entry.Key), Move(entry.Value)));
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Insert(Node* node)
    {
        EnsureCapacity();

        auto  index  = HashType{}(node->Entry.Key) % Capacity();
        auto& bucket = m_Buckets[index];

        auto  it     = bucket.PushBack(node);
        ++m_Size;

        return Iterator<>(this, index, it);
    }

    template <typename K, typename V, typename H>
    template <typename... Args>
    UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::TryEmplace(const K& key, Args&&... args)
    {
        auto it = Find(key);
        if (it != end()) return it;

        return Insert(new Node(key, Forward<Args>(args)...));
    }
    template <typename K, typename V, typename H>
    template <typename... Args>
    UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::TryEmplace(K&& k, Args&&... args)
    {
        auto key = Move(k);

        auto it  = Find(key);
        if (it != end()) return it;

        return Insert(new Node(Move(key), Forward<Args>(args)...));
    }
    template <typename K, typename V, typename H>
    template <typename... Args>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Emplace(Args&&... args)
    {
        auto node = new Node(Forward<Args>(args)...);

        // Keys are unique: keep the existing entry.
        auto it   = Find(node->Entry.Key);
        if (it != end())
        {
            delete node;
            return it;
        }
        return Insert(node);
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Erase(const K& key)
    {
        auto it = Find(key);
        if (it == end()) return end();

        return Erase(it);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Erase(Iterator<> it)
    {
        if (it.m_Map != this || it.m_BucketIndex >= Capacity()
            || it.ListIt == BucketType::Iterator::UniversalEnd())
            return end();

        Node* node   = it.ListIt.operator->();
        auto& bucket = m_Buckets[it.m_BucketIndex];
        auto  next   = bucket.Erase(it.ListIt);
        delete node;
        --m_Size;

        // Continue in the next non-empty bucket if this one is exhausted.
        Iterator<> nextIt(this, it.m_BucketIndex, next);
        nextIt.AdvanceToValid();
        return nextIt;
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Find(const K& key)
    {
        if (Empty()) return end();

        auto  index  = HashType{}(key) % Capacity();
        auto& bucket = m_Buckets[index];

        auto  it     = bucket.begin();
        while (it != bucket.end() && it->Entry.Key != key) ++it;

        if (it == bucket.end()) return end();
        return Iterator<>(this, index, it);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstIterator
    UnorderedMap<K, V, H>::Find(const K& key) const
    {
        if (Empty()) return end();

        usize index  = HashType{}(key) % Capacity();
        auto& bucket = m_Buckets[index];

        auto  it     = bucket.begin();
        while (it != bucket.end() && it->Entry.Key != key) ++it;

        if (it == bucket.end()) return end();
        return ConstIterator(this, index, it);
    }

}; // namespace Prism
