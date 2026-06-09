/*
 * Created by v1tr10l7 on 28.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/Limits.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Memory/Memory.hpp>

namespace Prism
{
    /**
     * @brief Dynamic array container similar to std::vector.
     *
     * Provides contiguous storage, automatic resizing, and standard container
     * functionality including iterators, insertion, removal, and element
     * access.
     *
     * @tparam T The type of element stored in the Vector.
     */
    template <typename T>
    class Vector
    {
      public:
        /** @brief Element type */
        using ValueType                = T;
        /** @brief Type used for size and indexing */
        using SizeType                 = usize;
        /** @brief Type used for pointer differences */
        using DifferenceType           = ptrdiff;
        /** @brief Reference to element */
        using ReferenceType            = ValueType&;
        /** @brief Const reference to element */
        using ConstReferenceType       = const ValueType&;
        /** @brief Pointer to element */
        using PointerType              = ValueType*;
        /** @brief Const pointer to element */
        using ConstPointerType         = const ValueType*;
        /** @brief Iterator type */
        using Iterator                 = T*;
        /** @brief Const iterator type */
        using ConstIterator            = const T*;
        /** @brief Reverse iterator type */
        using ReverseIterator          = Prism::ReverseIterator<Iterator>;
        /** @brief Const reverse iterator type */
        using ConstReverseIterator     = Prism::ReverseIterator<ConstIterator>;

        /** @brief Constructs an empty Vector. */
        constexpr Vector() PM_NOEXCEPT = default;

        /**
         * @brief Constructs a Vector with `size` default-initialized elements.
         * @param size Number of elements to allocate.
         */
        explicit Vector(usize size) { Resize(size); }

        /**
         * @brief Constructs a Vector with `size` copies of `value`.
         * @param size Number of elements.
         * @param value Value to copy into each element.
         */
        constexpr Vector(usize size, const T& value) { Resize(size, value); }

        /**
         * @brief Constructs a Vector with elements contained in range
         * `first`-`last`.
         * @param first Iterator pointing to the start of value range
         * @param value Iterator pointing to the end of value range
         */
        template <typename It>
        constexpr Vector(It start, It end)
        {
            auto size = Distance(start, end);
            Reserve(size);
            for (auto current = start; current != end; current = Next(current))
                EmplaceBack(*current);
        }

        /**
         * @brief Copy constructor.
         * @param other The Vector to copy from.
         */
        constexpr Vector(const Vector& other)
        {
            Reserve(other.m_Size);
            for (usize i = 0; i < other.m_Size; i++)
                new (m_Data + i) T(other.m_Data[i]);
            m_Size = other.m_Size;
        }

        /**
         * @brief Move constructor.
         * @param other The Vector to move from.
         */
        constexpr Vector(Vector&& other) PM_NOEXCEPT
            : m_Data(other.m_Data),
              m_Size(other.m_Size),
              m_Capacity(other.m_Capacity)
        {
            other.m_Data     = nullptr;
            other.m_Size     = 0;
            other.m_Capacity = 0;
        }

        /**
         * @brief Initializes the Vector with an initializer list.
         * @param init The initializer list of elements.
         */
        Vector(InitializerList<T> init)
        {
            Reserve(init.size());
            for (const auto& value : init)
            {
                new (m_Data + m_Size) T(value);
                ++m_Size;
            }
        }

        /** @brief Destructor. Releases allocated memory and destroys instances
         * safely. */
        constexpr ~Vector()
        {
            Clear();
            if (m_Data) { ::operator delete(m_Data, m_Capacity * sizeof(T)); }
        }

        /**
         * @brief Copy assignment operator.
         * @param other The Vector to copy from.
         * @return Reference to this Vector.
         */
        constexpr Vector& operator=(const Vector& other)
        {
            if (this != &other)
            {
                Vector temp(other);
                Swap(temp);
            }
            return *this;
        }

        /**
         * @brief Move assignment operator.
         * @param other The Vector to move from.
         * @return Reference to this Vector.
         */
        constexpr Vector& operator=(Vector&& other) PM_NOEXCEPT
        {
            if (this != &other)
            {
                Clear();
                if (m_Data) ::operator delete(m_Data, m_Capacity * sizeof(T));

                m_Data           = other.m_Data;
                m_Size           = other.m_Size;
                m_Capacity       = other.m_Capacity;

                other.m_Data     = nullptr;
                other.m_Size     = 0;
                other.m_Capacity = 0;
            }
            return *this;
        }

        /** @brief Checks if the Vector is empty. */
        PM_NODISCARD constexpr bool Empty() const PM_NOEXCEPT
        {
            return m_Size == 0;
        }
        /** @brief Returns the current number of elements. */
        constexpr SizeType Size() const PM_NOEXCEPT { return m_Size; }
        /** @brief Returns the current allocated capacity. */
        constexpr SizeType Capacity() const PM_NOEXCEPT { return m_Capacity; }

        /** @brief Returns the maximum size supported. */
        constexpr SizeType MaxSize() const PM_NOEXCEPT
        {
            return NumericLimits<DifferenceType>::Max() / sizeof(T);
        }

        /**
         * @brief Access element at index with bounds check.
         * @param pos Index to access.
         */
        constexpr ReferenceType At(SizeType pos)
        {
            assert(pos < m_Size);
            return m_Data[pos];
        }
        constexpr ConstReferenceType At(SizeType pos) const
        {
            assert(pos < m_Size);
            return m_Data[pos];
        }

        /** @brief Element access operator. */
        constexpr ReferenceType operator[](SizeType pos) { return m_Data[pos]; }
        constexpr ConstReferenceType operator[](SizeType pos) const
        {
            return m_Data[pos];
        }

        /** @brief Returns a reference to the first element. */
        constexpr ReferenceType Front()
        {
            assert(m_Size > 0);
            return m_Data[0];
        }
        constexpr ConstReferenceType Front() const
        {
            assert(m_Size > 0);
            return m_Data[0];
        }
        /** @brief Returns a reference to the last element. */
        constexpr ReferenceType Back()
        {
            assert(m_Size > 0);
            return m_Data[m_Size - 1];
        }
        constexpr ConstReferenceType Back() const
        {
            assert(m_Size > 0);
            return m_Data[m_Size - 1];
        }

        /** @brief Returns raw data pointer. */
        constexpr T*            Raw() PM_NOEXCEPT { return m_Data; }
        constexpr const T*      Raw() const PM_NOEXCEPT { return m_Data; }

        /** @brief Returns iterator to beginning. */
        constexpr Iterator      begin() PM_NOEXCEPT { return m_Data; }
        constexpr ConstIterator begin() const PM_NOEXCEPT { return m_Data; }
        constexpr ConstIterator cbegin() const PM_NOEXCEPT { return m_Data; }

        /** @brief Returns iterator to end. */
        constexpr Iterator      end() PM_NOEXCEPT { return m_Data + m_Size; }
        constexpr ConstIterator end() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }
        constexpr ConstIterator cend() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }

        /** @brief Returns reverse iterator to beginning. */
        constexpr ReverseIterator rbegin() PM_NOEXCEPT
        {
            return ReverseIterator(end());
        }
        constexpr ConstReverseIterator rbegin() const PM_NOEXCEPT
        {
            return ConstReverseIterator(end());
        }
        constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT
        {
            return ConstReverseIterator(end());
        }

        /** @brief Returns reverse iterator to end. */
        constexpr ReverseIterator rend() PM_NOEXCEPT
        {
            return ReverseIterator(begin());
        }
        constexpr ConstReverseIterator rend() const PM_NOEXCEPT
        {
            return ConstReverseIterator(begin());
        }
        constexpr ConstReverseIterator crend() const PM_NOEXCEPT
        {
            return ConstReverseIterator(begin());
        }

        /**
         * @brief Finds the first occurrence of a value starting from `start`.
         * @param value The value to find.
         * @param start Start index to search from.
         * @return Iterator to found element or end().
         */
        constexpr Iterator Find(const ValueType& value, usize start = 0)
        {
            for (usize i = start; i < m_Size; i++)
                if (m_Data[i] == value) return m_Data + i;
            return end();
        }

        constexpr ConstIterator Find(const ValueType& value,
                                     usize            start = 0) const
        {
            for (usize i = start; i < m_Size; i++)
                if (m_Data[i] == value) return m_Data + i;
            return end();
        }

        /** @brief Destroys actual living elements without freeing memory block.
         */
        constexpr void Clear() PM_NOEXCEPT
        {
            for (SizeType i = 0; i < m_Size; i++) m_Data[i].~T();
            m_Size = 0;
        }

        /** @brief Fills entire vector with `value`. */
        constexpr void Fill(const ValueType& value)
        {
            for (SizeType i = 0; i < m_Size; i++) m_Data[i] = value;
        }
        /**
         * @brief Fills a subrange with `value`.
         * @param value Value to assign.
         * @param start Start index.
         * @param count Number of elements to fill.
         */

        constexpr void FillRange(const ValueType& value, usize start,
                                 usize count = usize(-1))
        {
            assert(start < m_Size);
            if (count == usize(-1)) count = m_Size - start;
            assert(start + count <= m_Size);

            usize endIdx = start + count;
            for (usize i = start; i < endIdx; i++) m_Data[i] = value;
        }

        /**
         * @brief Inserts one or more elements at the specified position.
         * @param pos Iterator position to insert at.
         * @param value Value to insert.
         * @return Iterator to inserted element.
         */
        constexpr Iterator Insert(ConstIterator pos, const T& value)
        {
            // SizeType index = pos - m_Data;
            return Insert(pos, 1, value);
        }

        constexpr Iterator Insert(ConstIterator pos, T&& value)
        {
            SizeType index = pos - m_Data;
            assert(index <= m_Size);

            if (m_Size >= m_Capacity) Reserve(m_Capacity ? m_Capacity * 2 : 1);

            if (index < m_Size)
            {
                new (m_Data + m_Size) T(Move(m_Data[m_Size - 1]));
                for (SizeType i = m_Size - 1; i > index; --i)
                    m_Data[i] = Move(m_Data[i - 1]);
                m_Data[index] = Move(value);
            }
            else new (m_Data + index) T(Move(value));

            ++m_Size;
            return m_Data + index;
        }

        constexpr Iterator Insert(ConstIterator pos, SizeType count,
                                  const T& value)
        {
            SizeType index = pos - m_Data;
            assert(index <= m_Size);
            if (count == 0) return m_Data + index;

            Reserve(m_Size + count);

            // Construct new endpoints first, then shift down to protect
            // locations
            SizeType elements_to_move = m_Size - index;
            if (elements_to_move > 0)
            {
                for (SizeType i = 0; i < count; ++i)
                {
                    if (index + elements_to_move + i >= m_Size)
                    {
                        SizeType src_idx = index + elements_to_move - count + i;
                        new (m_Data + m_Size + i) T(Move(m_Data[src_idx]));
                    }
                }
                for (SizeType i = m_Size - 1; i >= index + count; --i)
                    m_Data[i] = Move(m_Data[i - count]);
                for (SizeType i = 0; i < count; ++i)
                {
                    if (index + i < m_Size) m_Data[index + i] = value;
                    else new (m_Data + index + i) T(value);
                }
            }
            else {
                for (SizeType i = 0; i < count; ++i)
                    new (m_Data + index + i) T(value);
            }

            m_Size += count;
            return m_Data + index;
        }

        /**
         * @brief Inserts a range of elements at `pos`.
         * @param pos Position to insert.
         * @param first Start of range.
         * @param last End of range.
         * @return Iterator to first inserted element.
         */
        template <typename InputIt>
        constexpr Iterator Insert(ConstIterator pos, InputIt first,
                                  InputIt last)
        {
            SizeType index = pos - m_Data;
            assert(index <= m_Size);
            SizeType count = Distance(first, last);
            if (count == 0) return m_Data + index;

            Reserve(m_Size + count);

            if (index < m_Size)
            {
                for (SizeType i = 0; i < count; ++i)
                {
                    new (m_Data + m_Size + i)
                        T(Move(m_Data[m_Size - count + i]));
                }
                for (SizeType i = m_Size - 1; i >= index + count; --i)
                    m_Data[i] = Move(m_Data[i - count]);
                SizeType i = 0;
                for (auto it = first; it != last; ++it, ++i)
                    m_Data[index + i] = *it;
            }
            else {
                SizeType i = 0;
                for (auto it = first; it != last; ++it, ++i)
                    new (m_Data + index + i) T(*it);
            }

            m_Size += count;
            return m_Data + index;
        }

        /**
         * @brief Inserts initializer list at position.
         * @param pos Position to insert at.
         * @param ilist Initializer list.
         * @return Iterator to first inserted element.
         */
        constexpr Iterator Insert(ConstIterator pos, InitializerList<T> ilist)
        {
            return Insert(pos, ilist.begin(), ilist.end());
        }

        /**
         * @brief Inserts a new element into the container directly before
         * `pos`.
         *
         * The element is constructed in-place, meaning no copy or move
         * operations are performed on the arguments. The container capacity is
         * automatically expanded if the current size equals capacity.
         *
         * @tparam Args... Varadic template types for the constructor arguments.
         * @param pos Iterator pointing to the position where the element will
         * be inserted.
         * @param args Arguments forwarded to the constructor of the element.
         * @return Iterator pointing to the emplaced element.
         */

        template <typename... Args>
        constexpr Iterator Emplace(ConstIterator pos, Args&&... args)
        {
            SizeType index = pos - m_Data;
            assert(index <= m_Size);

            if (m_Size >= m_Capacity) Reserve(m_Capacity ? m_Capacity * 2 : 1);

            if (index < m_Size)
            {
                new (m_Data + m_Size) T(Move(m_Data[m_Size - 1]));
                for (SizeType i = m_Size - 1; i > index; --i)
                    m_Data[i] = Move(m_Data[i - 1]);
                m_Data[index].~T();
            }

            new (m_Data + index) T(Forward<Args>(args)...);
            ++m_Size;
            return m_Data + index;
        }

        /**
         * @brief Removes element at specified iterator position.
         * @param pos Iterator to element.
         * @return Iterator to the next element.
         */
        constexpr Iterator Erase(Iterator pos)
        {
            assert(pos >= begin() && pos < end());
            usize index = pos - m_Data;

            for (usize i = index; i < (m_Size - 1); i++)
                m_Data[i] = Move(m_Data[i + 1]);

            m_Data[m_Size - 1].~T();
            --m_Size;

            return m_Data + index;
        }

        /**
         * @brief Adds an element to the end.
         * @param value Element to add.
         */
        void PushBack(const T& value)
        {
            if (m_Size >= m_Capacity) Reserve(m_Capacity ? m_Capacity * 2 : 1);
            new (m_Data + m_Size) T(value);
            ++m_Size;
        }

        void PushBack(T&& value)
        {
            if (m_Size >= m_Capacity) Reserve(m_Capacity ? m_Capacity * 2 : 1);
            new (m_Data + m_Size) T(Move(value));
            ++m_Size;
        }

        /**
         * @brief Constructs an element in place at the end.
         * @tparam Args Constructor arguments.
         * @return Reference to the new element.
         */
        template <typename... Args>
        T& EmplaceBack(Args&&... args)
        {
            if (m_Size >= m_Capacity) Reserve(m_Capacity ? m_Capacity * 2 : 1);
            T* element = new (m_Data + m_Size) T(Forward<Args>(args)...);
            ++m_Size;
            return *element;
        }

        /** @brief Removes the last element. */
        constexpr void PopBack()
        {
            if (m_Size == 0) return;
            m_Data[m_Size - 1].~T();
            --m_Size;
        }

        /**
         * @brief Removes the last element and returns it.
         * @return The removed element.
         */
        constexpr T PopBackElement()
        {
            assert(m_Size > 0);
            T value = Move(m_Data[m_Size - 1]);
            PopBack();
            return value;
        }

        /**
         * @brief Resizes the vector, value-initializing new elements.
         * @param count New size.
         */
        constexpr void Resize(SizeType count)
        {
            if (count < m_Size)
            {
                for (SizeType i = count; i < m_Size; ++i) { m_Data[i].~T(); }
            }
            else if (count > m_Size)
            {
                Reserve(count);
                for (SizeType i = m_Size; i < count; ++i) new (m_Data + i) T();
            }
            m_Size = count;
        }

        /**
         * @brief Resizes the vector and fills new elements with `value`.
         * @param count New size.
         * @param value Value to copy into new elements.
         */
        constexpr void Resize(SizeType count, const ValueType& value)
        {
            if (count < m_Size)
            {
                for (SizeType i = count; i < m_Size; ++i) { m_Data[i].~T(); }
            }
            else if (count > m_Size)
            {
                Reserve(count);
                for (SizeType i = m_Size; i < count; ++i)
                    new (m_Data + i) T(value);
            }
            m_Size = count;
        }

        /**
         * @brief Reserves space for at least `newCapacity` elements.
         * @param newCapacity The new capacity to allocate.
         */
        constexpr void Reserve(SizeType newCapacity)
        {
            if (newCapacity <= m_Capacity) return;

            // Allocate uninitialized storage memory layout explicitly
            T* newData
                = reinterpret_cast<T*>(::operator new(newCapacity * sizeof(T)));

            for (SizeType i = 0; i < m_Size; ++i)
            {
                new (newData + i) T(Move(m_Data[i]));
                m_Data[i].~T();
            }

            if (m_Data) { ::operator delete(m_Data, m_Capacity * sizeof(T)); }

            m_Data     = newData;
            m_Capacity = newCapacity;
        }

        /** @brief Reduces capacity to match current size. */
        constexpr void ShrinkToFit()
        {
            if (m_Size == m_Capacity) return;

            if (m_Size == 0)
            {
                if (m_Data) ::operator delete(m_Data, m_Capacity * sizeof(T));
                m_Data     = nullptr;
                m_Capacity = 0;
                return;
            }

            T* newData
                = reinterpret_cast<T*>(::operator new(m_Size * sizeof(T)));
            for (SizeType i = 0; i < m_Size; i++)
            {
                new (newData + i) T(Move(m_Data[i]));
                m_Data[i].~T();
            }

            ::operator delete(m_Data, m_Capacity * sizeof(T));
            m_Data     = newData;
            m_Capacity = m_Size;
        }

        /** @brief Equality comparison. */
        PM_NODISCARD friend constexpr bool operator==(const Vector<T>& lhs,
                                                      const Vector<T>& rhs)
        {
            if (lhs.m_Size != rhs.m_Size) return false;
            for (usize i = 0; i < lhs.m_Size; i++)
                if (lhs.m_Data[i] != rhs.m_Data[i]) return false;
            return true;
        }

        /** @brief Inequality comparison. */
        PM_NODISCARD friend constexpr bool operator!=(const Vector<T>& lhs,
                                                      const Vector<T>& rhs)
        {
            return !(lhs == rhs);
        }

        /** @brief Three-way comparison (by address). */
        PM_NODISCARD friend constexpr auto operator<=>(const Vector<T>& lhs,
                                                       const Vector<T>& rhs)
        {
            return lhs.m_Data <=> rhs.m_Data;
        }

      private:
        constexpr void Swap(Vector& other) PM_NOEXCEPT
        {
            auto* temp_data  = m_Data;
            auto  temp_size  = m_Size;
            auto  temp_cap   = m_Capacity;

            m_Data           = other.m_Data;
            m_Size           = other.m_Size;
            m_Capacity       = other.m_Capacity;

            other.m_Data     = temp_data;
            other.m_Size     = temp_size;
            other.m_Capacity = temp_cap;
        }

        PointerType m_Data     = nullptr;
        SizeType    m_Size     = 0;
        SizeType    m_Capacity = 0;
    };
} // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::Vector;
#endif
