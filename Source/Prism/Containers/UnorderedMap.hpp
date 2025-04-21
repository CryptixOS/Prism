/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/DoublyLinkedList.hpp>
#include <Prism/Containers/Vector.hpp>

#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/NonMovable.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Debug/Log.hpp>

namespace Prism
{
    template <typename T>
    concept IsImplicitlyDefaultConstructible
        = std::is_default_constructible_v<T>
       && std::is_trivially_default_constructible_v<T>;

    template <typename K, typename V>
    struct UnorderedMapNode
    {
        using Type      = UnorderedMapNode<K, V>;
        using KeyType   = K;
        using ValueType = V;

        K Key;
        V Value;

        constexpr explicit(IsImplicitlyDefaultConstructible<K>
                           && IsImplicitlyDefaultConstructible<V>)
            UnorderedMapNode()
                PM_NOEXCEPT(std::is_nothrow_default_constructible_v<K>&&
                                std::is_nothrow_default_constructible_v<V>)
            requires std::is_default_constructible_v<K>
                      && std::is_default_constructible_v<V>
            : Key()
            , Value()
        {
        }
        constexpr explicit(!std::is_convertible_v<const K&, const V&>())
            UnorderedMapNode(const K& key, const V& value) PM_NOEXCEPT(
                std::is_nothrow_constructible_v<const K&, const V&>())
            requires(std::is_constructible_v<const K&, const V&>())
            : Key(key)
            , Value(value)
        {
        }
        constexpr UnorderedMapNode(const Type&) = default;
        constexpr UnorderedMapNode(Type&&)      = default;
    };
    template <typename K, typename V, typename H = std::hash<K>>
    class UnorderedMap
    {
      public:
        using KeyType    = K;
        using ValueType  = V;
        using HashType   = H;
        using NodeType   = UnorderedMapNode<KeyType, ValueType>;
        using BucketType = DoublyLinkedList<NodeType>;

        using Iterator   = DoublyLinkedList<NodeType>::Iterator;

        UnorderedMap() { m_Buckets.Reserve(10); }

        bool     Empty() const PM_NOEXCEPT { return m_Buckets.Size() == 0; }
        usize    Size() const PM_NOEXCEPT { return m_Buckets.Size(); }

        // TODO(v1tr10l7): return iterator
        void     Insert(const K& key, V&& value);
        usize    Erase(const K& key);

        Iterator Find(const K& key);
        bool     Contains(const K& key) const
        {
            return Find(key) != m_Buckets.end();
        }

        void Rehash(usize count);

      private:
        Vector<BucketType> m_Buckets;
        usize              m_LoadFactorNumerator   = 1;
        usize              m_LoadFactorDenominator = 1;
        usize              m_MaxLoadFactor         = 1;
    };
} // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::UnorderedMap;
#endif

#include <Prism/Containers/UnorderedMap.inl>
