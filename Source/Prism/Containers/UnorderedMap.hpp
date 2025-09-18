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

#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/NonMovable.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Debug/Log.hpp>

namespace Prism
{
    /**
     * @brief Default hasher for keys used in UnorderedMap.
     *
     * @tparam K Type of the key.
     */
    template <typename K>
    struct Hasher
    {
        /**
         * @brief Hash function for the key.
         *
         * @param key The key to hash.
         * @return usize The resulting hash value.
         */
        constexpr usize operator()(K key)
        {
            auto hash = Hash<K>{}(key);

            return hash;
        }
    };

    /**
     * @brief A custom hash map implementation using intrusive lists.
     *
     * @tparam K Key type.
     * @tparam V Value type.
     * @tparam H Hash function object type (defaults to Hasher<K>).
     */
    template <typename K, typename V, typename H = Hasher<K>>
    class UnorderedMap
    {
      public:
        using KeyType   = K;
        using ValueType = V;
        using HashType  = H;

        /**
         * @brief Structure holding a key-value pair.
         */
        struct KeyValuePair
        {
            ///> The key
            KeyType   Key;
            ///> The value
            ValueType Value;

            /// @brief Default constructor.
            constexpr KeyValuePair() = default;
            /**
             * @brief Construct a key-value pair with lvalue reference.
             * @param key The key.
             * @param value The value.
             */
            constexpr KeyValuePair(const KeyType& key, const ValueType& value)
                : Key(key)
                , Value(value)
            {
            }
            /**
             * @brief Construct a key-value pair with rvalue reference for
             * value.
             * @param key The key.
             * @param value The value to move.
             */
            constexpr KeyValuePair(const KeyType& key, ValueType&& value)
                : Key(key)
                , Value(Move(value))
            {
            }
        };

        /**
         * @brief Node stored in each bucket of the hash map.
         */
        struct Node
        {
            using KeyType   = K;
            using ValueType = V;

            ///> The key-value pair.
            KeyValuePair            Entry;

            ///> Hook for intrusive list linking.
            IntrusiveListHook<Node> Hook;
            using List = IntrusiveList<Node>;

            /**
             * @brief Default constructor, enabled if key and value types are
             * default-constructible.
             */
            constexpr Node()
                requires(IsDefaultConstructibleV<KeyType>
                         && IsDefaultConstructibleV<ValueType>)
            {
            }
            /**
             * @brief Construct a node with lvalue reference.
             * @param key The key.
             * @param value The value.
             */
            constexpr Node(const KeyType& key, const ValueType& value)
                : Entry(key, value)
            {
            }
            /**
             * @brief Construct a node with rvalue reference for value.
             * @param key The key.
             * @param value The value to move.
             */
            constexpr Node(const KeyType& key, ValueType&& value)
                : Entry(key, Move(value))
            {
            }
        };

        using NodeType   = Node;
        using BucketType = IntrusiveList<Node>;

        /**
         * @brief Forward iterator for the UnorderedMap.
         *
         * @tparam Const Indicates if the iterator is const-qualified.
         */
        template <bool Const = false>
        struct Iterator
        {
            using MapPointerType
                = ConditionalType<Const, const UnorderedMap*, UnorderedMap*>;
            using EntryRefType
                = ConditionalType<Const, const KeyValuePair&, KeyValuePair&>;
            using EntryPointerType
                = ConditionalType<Const, KeyValuePair* const, KeyValuePair*>;

            constexpr Iterator(MapPointerType map, bool end)
                : m_Map(map)
            {
                if (end)
                {
                    m_BucketIndex = map->Capacity();
                    ListIt        = BucketType::Iterator::UniversalEnd();
                    return;
                }

                m_BucketIndex = 0;
                ListIt        = map->m_Buckets[m_BucketIndex].begin();
                while (ListIt == BucketType::Iterator::UniversalEnd()
                       && ++m_BucketIndex < map->Capacity())
                    ListIt = map->m_Buckets[m_BucketIndex].begin();
            }
            constexpr Iterator(const Iterator& other)
                : m_Map(other.m_Map)
                , m_BucketIndex(other.m_BucketIndex)
                , ListIt(other.ListIt)
            {
            }

            /**
             * @brief Constructs an iterator to the given bucket index in the
             * map.
             */
            // constexpr Iterator(
            //     ConditionalType<Const, const UnorderedMap*, UnorderedMap*>
            //     map, const usize index) : m_Map(map) , m_BucketIndex(index)
            //     , ListIt(BucketType::Iterator::UniversalEnd())
            // {
            //     if (m_BucketIndex >= m_Map->Capacity()) return;
            //
            //     ListIt = m_Map->m_Buckets[m_BucketIndex].begin();
            //     AdvanceToValid();
            // }
            /**
             * @brief Constructs an iterator pointing to a specific element in a
             * bucket.
             */
            constexpr Iterator(
                ConditionalType<Const, const UnorderedMap*, UnorderedMap*> map,
                usize index, typename BucketType::Iterator listIt)
                : m_Map(map)
                , m_BucketIndex(index)
                , ListIt(listIt)
            {
            }

            constexpr Iterator& operator=(const Iterator& other)
            {
                m_Map         = other.m_Map;
                m_BucketIndex = other.m_BucketIndex;
                ListIt        = other.ListIt;

                return *this;
            }

            /**
             * @brief Advances to the next valid bucket.
             */
            constexpr inline void AdvanceToValid()
            {
                if (m_BucketIndex >= m_Map->Capacity()) return;

                while (ListIt == m_Map->m_Buckets[m_BucketIndex].end())
                {
                    ++m_BucketIndex;
                    if (m_BucketIndex >= m_Map->Capacity()) break;

                    ListIt = m_Map->m_Buckets[m_BucketIndex].begin();
                }
            }

            /// @brief Dereference operator.
            constexpr EntryRefType operator*()
            {
#define PRISM_USE_INTRUSIVE_HASH_MAP 1
#if PRISM_USE_INTRUSIVE_HASH_MAP == 0
                return (*ListIt)->Entry;
#else
                return ListIt->Entry;
#endif
            }
            /// @brief Arrow operator.
            constexpr EntryPointerType operator->()
            {
#if PRISM_USE_INTRUSIVE_HASH_MAP == 0
                auto node = *ListIt;
                return &node->Entry;
#else
                return &ListIt->Entry;
#endif
            }

            /// @brief Prefix increment operator.
            constexpr Iterator& operator++()
            {
                ListIt++;
                while (ListIt == BucketType::Iterator::UniversalEnd()
                       && ++m_BucketIndex < m_Map->Capacity())
                    ListIt = m_Map->m_Buckets[m_BucketIndex].begin();

                return *this;

                ++ListIt;
                AdvanceToValid();

                return *this;
            }
            /// @brief Postfix increment operator.
            constexpr Iterator operator++(int)
            {
                Iterator copy(*this);
                operator++();
                return copy;

                auto it = *this;
                ++*this;

                return it;
            }

            /// @brief Equality operator.
            constexpr bool operator==(const Iterator& other) const
            {
                return m_Map == other.m_Map
                    && m_BucketIndex == other.m_BucketIndex
                    && ListIt == other.ListIt;
                // return (m_BucketIndex >= m_Map->Capacity()
                //         && other.m_BucketIndex >= m_Map->Capacity())
                //     || (m_BucketIndex == other.m_BucketIndex
                //         && ListIt == other.ListIt);
            }
            /// @brief Inequality operator.
            constexpr bool operator!=(const Iterator& other) const
            {
                return !(*this == other);
            }

            ConditionalType<Const, const UnorderedMap*, UnorderedMap*> m_Map
                = nullptr;
            usize                         m_BucketIndex = 0;
            typename BucketType::Iterator ListIt
                = BucketType::Iterator::UniversalEnd();
        };
        using ConstIterator        = Iterator<true>;
        using ReverseIterator      = ::Prism::ReverseIterator<Iterator<false>>;
        using ConstReverseIterator = ::Prism::ReverseIterator<Iterator<true>>;

        /**
         * @brief Constructs the unordered map with optional initial capacity.
         *
         * @param initialCapacity Number of buckets to preallocate.
         */
        constexpr UnorderedMap(usize initialCapacity = 8);
        /// @brief Destructor.
        constexpr ~UnorderedMap();

        /// @brief Checks if the map is empty.
        constexpr bool  Empty() const PM_NOEXCEPT { return m_Size == 0; }
        /// @brief Returns the number of key-value pairs in the map.
        constexpr usize Size() const PM_NOEXCEPT { return m_Size; }
        /// @brief Returns the number of buckets allocated.
        constexpr usize Capacity() const PM_NOEXCEPT
        {
            return m_Buckets.Size();
        }

        /**
         * @brief Returns the value associated with a key.
         * @param key The key to search for.
         * @return Value reference.
         * @throws If key does not exist.
         */
        constexpr ValueType&       At(const KeyType& key);
        /// @copydoc At(const KeyType&)
        constexpr const ValueType& At(const KeyType& key) const;
        /**
         * @brief Returns a reference to value associated with key, inserting if
         * not found.
         */
        constexpr ValueType&       operator[](const KeyType& key);
        /// @copydoc operator[](const KeyType&)
        constexpr ValueType&       operator[](KeyType&& key);

        /**
         * @brief Finds an element by key.
         *
         * @param key The key to search for.
         * @return Iterator to found element or end().
         */
        constexpr Iterator<>       Find(const K& key);
        /// @copydoc Find(const K&)
        constexpr ConstIterator    Find(const K& key) const;
        /**
         * @brief Checks if the key exists in the map.
         */
        constexpr bool             Contains(const K& key) const
        {
            return Find(key) != end();
        }

        /// @name Iterators
        /// @{
        constexpr Iterator<>           begin();
        constexpr ConstIterator        begin() const;
        constexpr ConstIterator        cbegin() const PM_NOEXCEPT;

        constexpr Iterator<>           end();
        constexpr ConstIterator        end() const;
        constexpr ConstIterator        cend() const PM_NOEXCEPT;

        // TODO(v1tr10l7): Reverse iterators

        constexpr ReverseIterator      rbegin();
        constexpr ConstReverseIterator rbegin() const;
        constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT;

        constexpr ReverseIterator      rend();
        constexpr ConstReverseIterator rend() const;
        constexpr ConstReverseIterator crend() const PM_NOEXCEPT;
        /// @}

        /// @brief Removes all elements from the map.
        constexpr void                 Clear() PM_NOEXCEPT;
        /**
         * @brief Rehashes the map to at least a new number of buckets.
         */
        constexpr void                 Rehash(usize count);
        /**
         * @brief Reserves capacity for at least a certain number of elements.
         */
        constexpr void                 Reserve(usize capacity);
        /**
         * @brief Ensures current load factor is under threshold, rehashing if
         * necessary.
         */
        constexpr void                 EnsureCapacity();

        /**
         * @brief Inserts a new key-value pair, if it doesn't exist.
         *  Otherwise it assigns the value to the existing element
         *
         *  @return iterator to the inserted value
         */
        constexpr Iterator<> InsertOrAssign(const K& key, const V& value);
        /// @copydoc InsertOrAssign(const K&, const V&)
        constexpr Iterator<> InsertOrAssign(const K& key, V&& value);
        /// @copydoc InsertOrAssign(const K&, const V&)
        constexpr Iterator<> InsertOrAssign(K&& key, V&& value);
        /**
         * @brief Inserts a new key-value pair, if it doesn't exist.
         *
         *  @return iterator to the inserted value
         */
        constexpr Iterator<> Insert(const KeyValuePair& value);
        /// @copydoc Insert(const KeyValuePair&)
        constexpr Iterator<> Insert(KeyValuePair&& value);
        /**
         * @brief Inserts a pre-allocated node into the map.
         */
        constexpr Iterator<> Insert(Node* node);

        /**
         * @brief Constructs a key-value pair in-place, if it doesn't already
         * eixst. If it exists, it does nothing
         *
         * @tparam Args Argument types for key/value.
         * @param args Arguments to forward to constructor.
         */
        template <typename... Args>
        Iterator<> TryEmplace(const K& key, Args&&... args);
        /// @copydoc TryEmplace(const K&, Args&&... args)
        template <typename... Args>
        Iterator<> TryEmplace(K&& key, Args&&... args);
        /**
         * @brief Constructs a key-value pair in-place.
         *
         * @tparam Args Argument types for key/value.
         * @param args Arguments to forward to constructor.
         */
        template <typename... Args>
        constexpr Iterator<> Emplace(Args&&... args);
        /**
         * @brief Removes a key-value pair by key.
         *
         * @param key Key to remove.
         * @return Iterator pointing to next valid element.
         */
        constexpr Iterator<> Erase(const K& key);
        /**
         * @brief Removes a key-value pair by iterator.
         *
         * @param it Iterator to element to be removed.
         * @return Iterator pointing to next valid element.
         */
        constexpr Iterator<> Erase(Iterator<> it);

      private:
        ///> Buckets holding linked lists of nodes.
        Vector<BucketType> m_Buckets;
        ///> Number of elements in the map.
        usize              m_Size                  = 0;
        ///> Load factor numerator.
        usize              m_LoadFactorNumerator   = 3;
        ///> Load factor denominator.
        usize              m_LoadFactorDenominator = 4;
        ///> Maximum load factor threshold.
        usize              m_MaxLoadFactor         = 1;
    };
} // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::UnorderedMap;
#endif
#include <Prism/Containers/UnorderedMap.inl>
