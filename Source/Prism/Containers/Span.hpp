/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/Limits.hpp>

namespace Prism
{
    inline constexpr usize DynamicExtent = NumericLimits<usize>::Max();
    template <typename T, usize Extent>
    class Span;

    namespace Details
    {
        template <typename T>
        inline constexpr bool IsSpanV = false;

        template <typename T, usize N>
        inline constexpr bool IsSpanV<Span<T, N>> = true;

        template <typename T>
        inline constexpr bool IsStdArray = false;

        template <typename T, usize N>
        inline constexpr bool IsStdArray<Array<T, N>> = true;

        template <usize ExtentSize>
        class ExtentStorage
        {
          public:
            constexpr ExtentStorage(usize) PM_NOEXCEPT {}

            static constexpr usize Extent() PM_NOEXCEPT { return ExtentSize; }
        };

        template <>
        class ExtentStorage<DynamicExtent>
        {
          public:
            constexpr ExtentStorage(usize extent) PM_NOEXCEPT
                : m_ExtentValue(extent)
            {
            }

            constexpr usize Extent() const PM_NOEXCEPT { return m_ExtentValue; }

          private:
            usize m_ExtentValue;
        };

        template <typename T>
        struct SpanPointer
        {
            T* const Address;
        };
    } // namespace Details

    template <typename T, usize Extent = DynamicExtent>
    class Span
    {
      public:
        using ElementType          = T;
        using ValueType            = RemoveCvType<T>;
        using SizeType             = usize;
        using PointerType          = T*;
        using ConstPointer         = const T*;
        using Reference            = T&;
        using ConstReference       = const T&;
        using Iterator             = PointerType;
        using ConstIterator        = ConstPointer;
        using ReverseIterator      = ::Prism::ReverseIterator<Iterator>;
        using ConstReverseIterator = ::Prism::ReverseIterator<ConstIterator>;

        template <typename U, usize ArrayExtent>
            requires(Extent == DynamicExtent || ArrayExtent == Extent)
        using IsCompatibleArray = IsArrayConvertible<T, U>;
        template <typename Ref>
        using IsCompatibleRef = IsArrayConvertible<T, RemoveReferenceType<Ref>>;

        constexpr Span() PM_NOEXCEPT = default;
        constexpr Span() noexcept
            requires(Extent == DynamicExtent || Extent == 0)
            : m_Data(nullptr)
            , m_Extent(Value<0>)
        {
        }

        template <typename It>
        explicit(Extent != DynamicExtent) constexpr Span(It first, usize count)
            requires IsCompatibleRef<IteratorReferenceType<It>>::Value
            : m_Data(ToAddress(first))
            , m_Extent(count)
        {
            if constexpr (Extent != DynamicExtent) assert(count == Extent);
        }

        template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
            requires IsCompatibleRef<IteratorReferenceType<It>>::Value
                      && (!IsConvertibleV<End, SizeType>)
        explicit(Extent != DynamicExtent) constexpr Span(It first, It last)
            PM_NOEXCEPT(PM_NOEXCEPT(last - first))
            : m_Data(ToAddress(first))
            , m_Extent(static_cast<SizeType>(last - first))
        {
        }
        template <usize N>
            requires(Extent == DynamicExtent || N == Extent)
        constexpr Span(TypeIdentityType<ElementType> (&arr)[N]) PM_NOEXCEPT
            : Span(static_cast<PointerType>(arr), N)
        {
        }
        template <typename U, usize N>
            requires IsCompatibleArray<U, N>::Value
        constexpr Span(Array<U, N>& arr) PM_NOEXCEPT
            : Span(static_cast<PointerType>(arr.Raw()), N)

        {
        }
        template <typename U, usize N>
            requires IsCompatibleArray<const U, N>::Value
        constexpr Span(const Array<U, N>& arr) PM_NOEXCEPT
            : Span(static_cast<PointerType>(arr.Raw()), N)

        {
        }

        template <typename Range>
            requires(!Details::IsSpanV<RemoveCvRefType<Range>>)
                 && (!Details::IsStdArray<RemoveCvRefType<Range>>)
                 && (!IsArrayV<RemoveCvRefType<Range>>)
                 && std::ranges::contiguous_range<Range>
                 && std::ranges::sized_range<Range>
                 && (std::ranges::borrowed_range<Range> || IsConstV<ValueType>)
                 && IsCompatibleRef<
                        std::ranges::range_reference_t<Range>>::value
        constexpr explicit(Extent != DynamicExtent) Span(Range&& range)
            PM_NOEXCEPT(PM_NOEXCEPT(std::ranges::data(range))
                        && PM_NOEXCEPT(std::ranges::size(range)))
            : Span(std::ranges::data(range), std::ranges::size(range))
        {
            if constexpr (Extent != DynamicExtent)
                assert(std::ranges::size(range) == Extent);
        }

        explicit(Extent != DynamicExtent) constexpr Span(
            std::initializer_list<ValueType> il) PM_NOEXCEPT
            : m_Data(il.begin()),
              m_Extent(il.size())
        {
        }

        template <typename U, usize N>
            requires(Extent == DynamicExtent || N == DynamicExtent
                     || Extent == N)
                     && (IsArrayConvertible<T, U>::Value)
        constexpr explicit(Extent != DynamicExtent && N == DynamicExtent)
            Span(const Span<U, N>& s) PM_NOEXCEPT : m_Extent(s.Size()),
                                                    m_Data(s.Raw())
        {
            if constexpr (Extent != DynamicExtent) assert(s.Size() == Extent);
        }
        constexpr Span(const Span& other) PM_NOEXCEPT               = default;

        constexpr Span&    operator=(const Span& other) PM_NOEXCEPT = default;

        constexpr Iterator begin() const PM_NOEXCEPT
        {
            return Iterator(m_Data);
        }
        constexpr ConstIterator cbegin() const PM_NOEXCEPT
        {
            return Iterator(m_Data);
        }

        constexpr Iterator end() const PM_NOEXCEPT
        {
            return Iterator(m_Data + Size());
        }
        constexpr ConstIterator cend() const PM_NOEXCEPT
        {
            return ConstIterator(m_Data + Size());
        }

        constexpr ReverseIterator rbegin() const PM_NOEXCEPT
        {
            return ReverseIterator(m_Data + Size());
        }
        constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT
        {
            return ConstReverseIterator(m_Data + Size());
        }

        constexpr ReverseIterator rend() const PM_NOEXCEPT
        {
            return ReverseIterator(m_Data);
        }
        constexpr ConstReverseIterator crend() const PM_NOEXCEPT
        {
            return ConstReverseIterator(m_Data);
        }

        constexpr Reference Front() const
        {
            assert(!Empty());
            return m_Data[0];
        }
        constexpr Reference Back() const
        {
            assert(!Empty());
            return m_Data[Size() - 1];
        }
        constexpr Reference At(SizeType pos) const
        {
            assert(pos < Size());
            return m_Data[pos];
        }
        constexpr Reference operator[](SizeType index) const
        {
            return At(index);
        }
        constexpr PointerType Raw() const PM_NOEXCEPT { return m_Data; }

        constexpr SizeType    Size() const PM_NOEXCEPT
        {
            return m_Extent.Extent();
        }
        constexpr SizeType SizeInBytes() const PM_NOEXCEPT
        {
            return Size() * sizeof(T);
        }
        constexpr bool Empty() const PM_NOEXCEPT { return Size() == 0; }

        template <usize Count>
        constexpr Span<ElementType, Count> First() const
        {
            return {m_Data, Size()};
        }
        constexpr Span<ElementType, DynamicExtent> First(SizeType count) const
        {
            assert(count <= Size());
            return {m_Data, count};
        }

        template <usize Count>
        constexpr Span<ElementType, Count> Last() const
        {
            return {m_Data + Size() - Count, Count};
        }
        constexpr Span<ElementType, DynamicExtent> Last(SizeType count) const
        {
            assert(count <= Size());
            return {m_Data + Size() - count, count};
        }
        template <usize Offset, usize Count = DynamicExtent>
        constexpr Span<ElementType> SubSpan() const
        {
            assert(Offset + Count <= Size());
            return {m_Data + Offset, Count};
        }
        constexpr Span<ElementType, DynamicExtent>
        SubSpan(SizeType offset, SizeType count = DynamicExtent) const
        {
            assert(offset + count <= Size());
            return {m_Data + offset, count};
        }

      private:
        template <typename, usize>
        friend class Span;

        template <usize N>
        static inline constexpr IntegralConstant<usize, N> Value{};

        using SizedPointer = Details::SpanPointer<T>;

        PM_ALWAYS_INLINE
        constexpr explicit Span(SizedPointer addr) PM_NOEXCEPT
            requires(Extent != DynamicExtent)
            : m_Data(addr.Address)
            , m_Extent(Value<Extent>)
        {
        }

        PointerType                                          m_Data = nullptr;
        [[no_unique_address]] Details::ExtentStorage<Extent> m_Extent;
    };

    template <typename T, usize Extent>
    Span(T (&)[Extent]) -> Span<T, Extent>;

    template <typename T, usize Extent>
    Span(Array<T, Extent>&) -> Span<T, Extent>;

    template <typename T, usize Extent>
    Span(const Array<T, Extent>&) -> Span<const T, Extent>;

    template <std::contiguous_iterator It, typename End>
    Span(It, End) -> Span<RemoveReferenceType<IteratorReferenceType<It>>>;

    template <std::ranges::contiguous_range Range>
    Span(Range&&)
        -> Span<RemoveReferenceType<std::ranges::range_reference_t<Range&>>>;
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::DynamicExtent;
using Prism::Span;
#endif
