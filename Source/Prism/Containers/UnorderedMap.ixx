/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>

#include <Prism/Debug/Log.hpp>
#include <cassert>
#include <Prism/Core/Compiler.hpp>

export module Prism.Containers.UnorderedMap;

export import Prism.Containers.DoublyLinkedList;
export import Prism.Containers.IntrusiveListV3;
export import Prism.Containers.Vector;
export import Prism.Core.Iterator;
export import Prism.Core.NonCopyable;
export import Prism.Core.NonMovable;
export import Prism.Debug.Log;
export import Prism.Core.Core;
export import Prism.Memory.Ref;
export import Prism.Memory.Scope;
export import Prism.Core.Limits;
export import Prism.Memory.Memory;

export extern "C++"
{
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
            constexpr usize operator()(const K& key) const
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
                KeyValuePair                Entry;

                ///> Hook for intrusive list linking.
                V3::IntrusiveListHook<Node> Hook;

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
            using BucketType = V3::IntrusiveList<Node>;

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
                    = ConditionalType<Const, const KeyValuePair*, KeyValuePair*>;
                using ListIteratorType
                    = ConditionalType<Const, typename BucketType::ConstIterator,
                                      typename BucketType::Iterator>;

                constexpr Iterator(MapPointerType map, bool end)
                    : m_Map(map)
                {
                    if (end)
                    {
                        m_BucketIndex = map->Capacity();
                        ListIt        = ListIteratorType{};
                        return;
                    }

                    m_BucketIndex = 0;
                    if (map->Capacity() == 0) return;
                    ListIt = map->m_Buckets[m_BucketIndex].begin();
                    while (ListIt == ListIteratorType{}
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
                //     , ListIt(ListIteratorType{})
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
                    usize index, ListIteratorType listIt)
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
                    while (ListIt == ListIteratorType{}
                           && m_BucketIndex < m_Map->Capacity())
                    {
                        ++m_BucketIndex;
                        if (m_BucketIndex >= m_Map->Capacity()) break;

                        ListIt = m_Map->m_Buckets[m_BucketIndex].begin();
                    }
                }

                /// @brief Dereference operator.
                constexpr EntryRefType operator*() const
                {
#define PRISM_USE_INTRUSIVE_HASH_MAP 1
#if PRISM_USE_INTRUSIVE_HASH_MAP == 0
                    return (*ListIt)->Entry;
#else
                    return ListIt->Entry;
#endif
                }
                /// @brief Arrow operator.
                constexpr EntryPointerType operator->() const
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
                    while (ListIt == ListIteratorType{}
                           && ++m_BucketIndex < m_Map->Capacity())
                        ListIt = m_Map->m_Buckets[m_BucketIndex].begin();

                    return *this;
                }
                /// @brief Postfix increment operator.
                constexpr Iterator operator++(int)
                {
                    Iterator copy(*this);
                    operator++();
                    return copy;
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
                usize            m_BucketIndex = 0;
                ListIteratorType ListIt{};
            };
            using ConstIterator        = Iterator<true>;

            /**
             * @brief Constructs the unordered map with optional initial capacity.
             *
             * @param initialCapacity Number of buckets to preallocate.
             */
            constexpr UnorderedMap(usize initialCapacity = 8);
            /// @brief Deep copy.
            constexpr UnorderedMap(const UnorderedMap& other);
            /// @brief Takes other's nodes; other is left empty but usable.
            constexpr UnorderedMap(UnorderedMap&& other) PM_NOEXCEPT;
            constexpr UnorderedMap& operator=(const UnorderedMap& other);
            constexpr UnorderedMap& operator=(UnorderedMap&& other) PM_NOEXCEPT;
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
            /// @brief Const access can't insert: the key must exist (like At()).
            constexpr const ValueType& operator[](const KeyType& key) const;
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

            // TODO(v1tr10l7): Reverse iterators (needs a bidirectional
            // Iterator; the previous stubs could never compile).
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

#if PRISM_USE_NAMESPACE != 0
    using Prism::UnorderedMap;
#endif

    namespace Prism
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
                while (Node* node = bucket.TakeFront()) delete node;

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
                while (Node* node = bucket.TakeFront())
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
                || it.ListIt == typename BucketType::Iterator{})
                return end();

            Node* node   = it.ListIt.Node();
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
}
