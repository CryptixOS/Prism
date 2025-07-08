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
    constexpr UnorderedMap<K, V, H>::UnorderedMap(usize initialCapacity)
    {
        m_Buckets.Resize(initialCapacity);
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::~UnorderedMap()
    {
        Clear();
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

        it = InsertOrAssign(key, V());
        return it->Value;
    }
    template <typename K, typename V, typename H>
    constexpr V& UnorderedMap<K, V, H>::operator[](KeyType&& k)
    {
        auto key = Move(k);

        auto it  = Find(key);
        if (it != end()) return it->Value;

        return InsertOrAssign(key, V())->Value;
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
    constexpr UnorderedMap<K, V, H>::ReverseIterator
    UnorderedMap<K, V, H>::rbegin()
    {
        end();
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstReverseIterator
    UnorderedMap<K, V, H>::rbegin() const
    {
        return end();
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstReverseIterator
    UnorderedMap<K, V, H>::crbegin() const PM_NOEXCEPT
    {
        return cend();
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ReverseIterator
    UnorderedMap<K, V, H>::rend()
    {
        return begin();
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstReverseIterator
    UnorderedMap<K, V, H>::rend() const
    {
        return begin();
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::ConstReverseIterator
    UnorderedMap<K, V, H>::crend() const PM_NOEXCEPT
    {
        return cbegin();
    }

    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::Clear() PM_NOEXCEPT
    {
        for (auto& bucket : m_Buckets)
        {
#if PRISM_USE_INTRUSIVE_HASH_MAP == 0
            for (const auto& node : bucket) delete &node;
            bucket.Clear();
#else
            auto  next    = bucket.Head();
            Node* current = nullptr;
            while (next)
            {
                current = next;
                next    = current->Hook.Next;

                current->Hook.Unlink(current);
                delete current;
            }
#endif
        }

        m_Buckets.Clear();
        m_Buckets.ShrinkToFit();
        m_Size = 0;
    }
    template <typename K, typename V, typename H>
    constexpr void UnorderedMap<K, V, H>::Rehash(usize count)
    {
        if (Capacity() * m_LoadFactorDenominator
            < Capacity() * m_LoadFactorNumerator)
            return;

#if PRISM_USE_INTRUSIVE_HASH_MAP != 0
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

        oldTable.Clear();
        oldTable.ShrinkToFit();
#else

        auto oldBuckets = Move(m_Buckets);
        assert(m_Buckets.Empty());
        m_Buckets.Resize(count);
        m_Size = 0;

        for (auto& bucket : oldBuckets)
        {
            auto current = bucket.Head();
            while (current)
            {
                auto newIndex = HashType{}(current->Entry.Key) % count;
                m_Buckets[newIndex].PushBack(current);
                ++m_Size;

                current = current->Hook.Next;
            }
            for (const auto& node : bucket)
            {
                auto newIndex = HashType{}(node.Entry.Key) % count;
                m_Buckets[newIndex].PushBack(&node);
                ++m_Size;
            }
            bucket.Clear();
        }
        oldBuckets.Clear();
#endif
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

        return Insert(entry);
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
        // TODO(v1tr10l7): What if there exists a node with the same key
        auto node = new Node(Forward<Args>(args)...);
        return Insert(node);
    }

    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Erase(const K& key)
    {
        if (Empty()) return end();

        auto  index  = HashType{}(key) % Capacity();
        auto& bucket = m_Buckets[index];

        auto  it     = bucket.begin();
        while (it != bucket.end() && it->Entry.Key != key) ++it;

        if (it == bucket.end()) return end();
        --m_Size;

        auto node    = it.operator->();
        auto nextPos = bucket.Erase(it);
        delete node;

        auto nextIt
            = Iterator<>(this, index, typename BucketType::Iterator(nextPos));
        return index < Capacity() ? nextIt : end();
    }
    template <typename K, typename V, typename H>
    constexpr UnorderedMap<K, V, H>::Iterator<>
    UnorderedMap<K, V, H>::Erase(Iterator<> it)
    {
        if (Empty()) return end();

        --m_Size;
        auto  node    = it.ListIt.operator->();
        usize index   = HashType{}(node->Entry.Key) % Capacity();
        auto  bucket  = m_Buckets[index];

        auto  nextPos = bucket.Erase(it.ListIt);
        delete node;

        auto nextIt
            = Iterator<>(this, index, typename BucketType::Iterator(nextPos));
        return index < Capacity() ? nextIt : end();
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
