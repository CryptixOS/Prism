/*
 * Created by v1tr10l7 on 12.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Core.hpp>

/**
 * PRISM_STD_INTEROP lets Prism's iterator and range concepts understand the
 * standard library's iterators and ranges (std iterator tags,
 * std::ranges::enable_borrowed_range / enable_view), and makes Prism views
 * std views too. It's on by default for hosted builds that have <iterator>
 * and <ranges>, off for CryptixOS. Define it to 0 or 1 to override.
 */
#ifndef PRISM_STD_INTEROP
    #if PRISM_TARGET_CRYPTIX == 0 && PrismHasInclude(<iterator>)               \
        && PrismHasInclude(<ranges>)
        #define PRISM_STD_INTEROP 1
    #else
        #define PRISM_STD_INTEROP 0
    #endif
#endif
#if PRISM_STD_INTEROP != 0
    #include <iterator>
    #include <ranges>
#endif

namespace Prism
{
    enum class IterationResult
    {
        eBreak    = 0x00,
        eContinue = 0x01,
    };

    struct InputIteratorTag
    {
    };
    struct OutputIteratorTag
    {
    };
    struct ForwardIteratorTag : InputIteratorTag
    {
    };
    struct BidirectionalIteratorTag : ForwardIteratorTag
    {
    };
    struct RandomAccessIteratorTag : BidirectionalIteratorTag
    {
    };
    struct ContiguousIteratorTag : RandomAccessIteratorTag
    {
    };

    template <typename Iterator>
    struct IteratorTraits;
    // Partial specialization for pointer types
    template <typename T>
    struct IteratorTraits<T*>
    {
      public:
        using DifferenceType   = ptrdiff;
        using ValueType        = T;
        using Pointer          = T*;
        using Reference        = T&;
        using IteratorCategory = RandomAccessIteratorTag;
    };
    // Partial specialization for const pointer types
    template <typename T>
    struct IteratorTraits<const T*>
    {
      public:
        using DifferenceType   = ptrdiff;
        using ValueType        = T;
        using Pointer          = const T*;
        using Reference        = const T&;
        using IteratorCategory = RandomAccessIteratorTag;
    };

    struct DefaultSentinelType
    {
    };

    /// A default sentinel value.
    inline constexpr DefaultSentinelType DefaultSentinel{};

    namespace Details
    {
        template <typename T>
        using WithRef = T&;

        template <typename T>
        concept CanReference = requires { typename WithRef<T>; };

        template <typename T>
        concept Dereferenceable = requires(T& t) {
            { *t } -> CanReference;
        };
    } // namespace Details

    template <Details::Dereferenceable T>
    using IteratorReferenceType = decltype(*DeclVal<T&>());

    // ==========================================================================
    // Iterator associated types (std::iter_value_t, std::iter_difference_t,
    // ...). Prism-style member types (ValueType, DifferenceType,
    // IteratorConcept, IteratorCategory) are used first, then the standard
    // library spellings (value_type, ...).
    namespace Details
    {
        struct NoType
        {
        };

        // Maps an iterator tag to the matching Prism tag, or NoType.
        template <typename Tag>
        constexpr auto NormalizeIteratorTag()
        {
            if constexpr (IsBaseOfV<InputIteratorTag, Tag>
                          || IsBaseOfV<OutputIteratorTag, Tag>)
                return TypeIdentity<Tag>{};
#if PRISM_STD_INTEROP != 0
            else if constexpr (IsBaseOfV<std::contiguous_iterator_tag, Tag>)
                return TypeIdentity<ContiguousIteratorTag>{};
            else if constexpr (IsBaseOfV<std::random_access_iterator_tag, Tag>)
                return TypeIdentity<RandomAccessIteratorTag>{};
            else if constexpr (IsBaseOfV<std::bidirectional_iterator_tag, Tag>)
                return TypeIdentity<BidirectionalIteratorTag>{};
            else if constexpr (IsBaseOfV<std::forward_iterator_tag, Tag>)
                return TypeIdentity<ForwardIteratorTag>{};
            else if constexpr (IsBaseOfV<std::input_iterator_tag, Tag>)
                return TypeIdentity<InputIteratorTag>{};
            else if constexpr (IsBaseOfV<std::output_iterator_tag, Tag>)
                return TypeIdentity<OutputIteratorTag>{};
#endif
            else return NoType{};
        }

        template <typename T>
        constexpr auto ObjectValue()
        {
            if constexpr (IsObjectV<T>) return TypeIdentity<RemoveCvType<T>>{};
            else return NoType{};
        }

        template <typename I>
        constexpr auto SelectIteratorValue()
        {
            if constexpr (IsPointerV<I>)
                return ObjectValue<RemovePointerType<I>>();
            else if constexpr (IsArrayV<I>)
                return TypeIdentity<RemoveCvType<RemoveExtentType<I>>>{};
            else if constexpr (requires { typename I::ValueType; })
                return ObjectValue<typename I::ValueType>();
            else if constexpr (requires { typename I::value_type; })
                return ObjectValue<typename I::value_type>();
            else if constexpr (requires { typename I::ElementType; })
                return ObjectValue<typename I::ElementType>();
            else if constexpr (requires { typename I::element_type; })
                return ObjectValue<typename I::element_type>();
            else return NoType{};
        }

        template <typename I>
        constexpr auto SelectIteratorDifference()
        {
            if constexpr (IsPointerV<I>)
            {
                if constexpr (IsObjectV<RemovePointerType<I>>)
                    return TypeIdentity<ptrdiff>{};
                else return NoType{};
            }
            else if constexpr (requires { typename I::DifferenceType; })
                return TypeIdentity<typename I::DifferenceType>{};
            else if constexpr (requires { typename I::difference_type; })
                return TypeIdentity<typename I::difference_type>{};
            else if constexpr (requires(const I& a, const I& b) {
                                   { a - b } -> Integral;
                               })
                return TypeIdentity<MakeSignedType<decltype(
                    DeclVal<const I&>() - DeclVal<const I&>())>>{};
            else return NoType{};
        }

        // ITER_CONCEPT: defaults to random access, the concepts then check
        // which operations the iterator really has.
        template <typename I>
        constexpr auto SelectIteratorConcept()
        {
            if constexpr (IsPointerV<I>) return TypeIdentity<ContiguousIteratorTag>{};
            else if constexpr (requires { typename I::IteratorConcept; })
                return NormalizeIteratorTag<typename I::IteratorConcept>();
            else if constexpr (requires { typename I::IteratorCategory; })
                return NormalizeIteratorTag<typename I::IteratorCategory>();
            else if constexpr (requires { typename I::iterator_concept; })
                return NormalizeIteratorTag<typename I::iterator_concept>();
            else if constexpr (requires { typename I::iterator_category; })
                return NormalizeIteratorTag<typename I::iterator_category>();
            else return TypeIdentity<RandomAccessIteratorTag>{};
        }

        /// std::to_address
        template <typename T>
        PM_ALWAYS_INLINE constexpr T* ToAddressOf(T* pointer) PM_NOEXCEPT
        {
            return pointer;
        }
        template <typename P>
            requires requires(const P& p) { p.operator->(); }
        PM_ALWAYS_INLINE constexpr auto ToAddressOf(const P& pointer) PM_NOEXCEPT
        {
            return Details::ToAddressOf(pointer.operator->());
        }
    } // namespace Details

    template <typename I>
    using IteratorValueType = typename decltype(Details::SelectIteratorValue<
                                                RemoveCvRefType<I>>())::Type;
    template <typename I>
    using IteratorDifferenceType =
        typename decltype(Details::SelectIteratorDifference<
                          RemoveCvRefType<I>>())::Type;
    /// The iterator tag that describes I (std's ITER_CONCEPT).
    template <typename I>
    using IteratorConceptType = typename decltype(Details::SelectIteratorConcept<
                                                  RemoveCvRefType<I>>())::Type;

    namespace Ranges
    {
        namespace Impl
        {
            struct IterMoveFn
            {
                template <typename I>
                    requires Details::Dereferenceable<RemoveReferenceType<I>>
                constexpr decltype(auto) operator()(I&& it) const
                {
                    if constexpr (IsLValueReferenceV<decltype(*Forward<I>(it))>)
                        return Move(*Forward<I>(it));
                    else return *Forward<I>(it);
                }
            };
        } // namespace Impl

        /// std::ranges::iter_move
        inline constexpr Impl::IterMoveFn IterMove{};
    } // namespace Ranges

    template <typename I>
        requires Details::Dereferenceable<I>
    using IteratorRValueReferenceType
        = decltype(Ranges::IterMove(DeclVal<I&>()));

    // ==========================================================================
    // Iterator concepts (std::input_iterator, ...)
    namespace Details
    {
        template <typename I>
        concept IndirectlyReadableImpl = requires(const I i) {
            typename IteratorValueType<I>;
            typename IteratorReferenceType<I>;
            typename IteratorRValueReferenceType<I>;
            { *i } -> SameAs<IteratorReferenceType<I>>;
            { Ranges::IterMove(i) } -> SameAs<IteratorRValueReferenceType<I>>;
        };
    } // namespace Details

    template <typename I>
    concept IndirectlyReadable
        = Details::IndirectlyReadableImpl<RemoveCvRefType<I>>;

    template <IndirectlyReadable I>
    using IteratorCommonReferenceType
        = CommonReferenceType<IteratorReferenceType<I>, IteratorValueType<I>&>;

    template <typename Out, typename T>
    concept IndirectlyWritable = requires(Out&& o, T&& t) {
        *o                    = Forward<T>(t);
        *Forward<Out>(o)      = Forward<T>(t);
        const_cast<const IteratorReferenceType<Out>&&>(*o) = Forward<T>(t);
        const_cast<const IteratorReferenceType<Out>&&>(*Forward<Out>(o))
            = Forward<T>(t);
    };

    template <typename I>
    concept WeaklyIncrementable = Movable<I> && requires(I i) {
        typename IteratorDifferenceType<I>;
        requires SignedIntegral<IteratorDifferenceType<I>>;
        { ++i } -> SameAs<I&>;
        i++;
    };
    template <typename I>
    concept Incrementable = Regular<I> && WeaklyIncrementable<I>
                         && requires(I i) {
                                { i++ } -> SameAs<I>;
                            };

    template <typename I>
    concept InputOrOutputIterator = requires(I i) {
        { *i } -> Details::CanReference;
    } && WeaklyIncrementable<I>;

    template <typename S, typename I>
    concept SentinelFor = SemiRegular<S> && InputOrOutputIterator<I>
                       && Details::WeaklyEqualityComparableWith<S, I>;

    /// Specialize to true to opt S/I out of SizedSentinelFor.
    template <typename S, typename I>
    inline constexpr bool DisableSizedSentinelFor = false;

    template <typename S, typename I>
    concept SizedSentinelFor
        = SentinelFor<S, I>
       && !DisableSizedSentinelFor<RemoveCvType<S>, RemoveCvType<I>>
       && requires(const I& i, const S& s) {
              { s - i } -> SameAs<IteratorDifferenceType<I>>;
              { i - s } -> SameAs<IteratorDifferenceType<I>>;
          };

    template <typename I>
    concept InputIterator
        = InputOrOutputIterator<I> && IndirectlyReadable<I>
       && requires { typename IteratorConceptType<I>; }
       && DerivedFrom<IteratorConceptType<I>, InputIteratorTag>;

    template <typename I, typename T>
    concept OutputIterator
        = InputOrOutputIterator<I> && IndirectlyWritable<I, T>
       && requires(I i, T&& t) { *i++ = Forward<T>(t); };

    template <typename I>
    concept ForwardIterator
        = InputIterator<I>
       && DerivedFrom<IteratorConceptType<I>, ForwardIteratorTag>
       && Incrementable<I> && SentinelFor<I, I>;

    template <typename I>
    concept BidirectionalIterator
        = ForwardIterator<I>
       && DerivedFrom<IteratorConceptType<I>, BidirectionalIteratorTag>
       && requires(I i) {
              { --i } -> SameAs<I&>;
              { i-- } -> SameAs<I>;
          };

    template <typename I>
    concept RandomAccessIterator
        = BidirectionalIterator<I>
       && DerivedFrom<IteratorConceptType<I>, RandomAccessIteratorTag>
       && TotallyOrdered<I> && SizedSentinelFor<I, I>
       && requires(I i, const I j, const IteratorDifferenceType<I> n) {
              { i += n } -> SameAs<I&>;
              { j + n } -> SameAs<I>;
              { n + j } -> SameAs<I>;
              { i -= n } -> SameAs<I&>;
              { j - n } -> SameAs<I>;
              { j[n] } -> SameAs<IteratorReferenceType<I>>;
          };

    template <typename I>
    concept ContiguousIterator
        = RandomAccessIterator<I>
       && DerivedFrom<IteratorConceptType<I>, ContiguousIteratorTag>
       && IsLValueReferenceV<IteratorReferenceType<I>>
       && SameAs<IteratorValueType<I>,
                 RemoveCvRefType<IteratorReferenceType<I>>>
       && requires(const I& i) {
              {
                  Details::ToAddressOf(i)
              } -> SameAs<AddPointerType<IteratorReferenceType<I>>>;
          };

    // Callable concepts over iterators (std::indirect_unary_predicate, ...)
    template <typename F, typename I>
    concept IndirectlyUnaryInvocable
        = IndirectlyReadable<I> && CopyConstructible<F>
       && Invocable<F&, IteratorValueType<I>&>
       && Invocable<F&, IteratorReferenceType<I>>;
    template <typename F, typename I>
    concept IndirectlyRegularUnaryInvocable
        = IndirectlyReadable<I> && CopyConstructible<F>
       && RegularInvocable<F&, IteratorValueType<I>&>
       && RegularInvocable<F&, IteratorReferenceType<I>>;
    template <typename F, typename I>
    concept IndirectUnaryPredicate
        = IndirectlyReadable<I> && CopyConstructible<F>
       && Predicate<F&, IteratorValueType<I>&>
       && Predicate<F&, IteratorReferenceType<I>>;

    /// A sentinel that never compares equal to an iterator.
    struct UnreachableSentinelType
    {
        template <WeaklyIncrementable I>
        friend constexpr bool operator==(UnreachableSentinelType,
                                         const I&) PM_NOEXCEPT
        {
            return false;
        }
    };
    inline constexpr UnreachableSentinelType UnreachableSentinel{};

    // ==========================================================================
    // IteratorTraits: an iterator's member types when it has them, otherwise
    // the types derived above. Works for any iterator, including the standard
    // library's.
    namespace Details
    {
        template <typename I>
        constexpr auto TraitsValue()
        {
            if constexpr (requires { typename I::ValueType; })
                return TypeIdentity<typename I::ValueType>{};
            else return TypeIdentity<IteratorValueType<I>>{};
        }
        template <typename I>
        constexpr auto TraitsDifference()
        {
            if constexpr (requires { typename I::DifferenceType; })
                return TypeIdentity<typename I::DifferenceType>{};
            else return TypeIdentity<IteratorDifferenceType<I>>{};
        }
        template <typename I>
        constexpr auto TraitsPointer()
        {
            if constexpr (requires { typename I::Pointer; })
                return TypeIdentity<typename I::Pointer>{};
            else if constexpr (requires { typename I::pointer; })
                return TypeIdentity<typename I::pointer>{};
            else if constexpr (requires(I& i) { i.operator->(); })
                return TypeIdentity<decltype(DeclVal<I&>().operator->())>{};
            else return TypeIdentity<void>{};
        }
        template <typename I>
        constexpr auto TraitsReference()
        {
            if constexpr (requires { typename I::Reference; })
                return TypeIdentity<typename I::Reference>{};
            else return TypeIdentity<IteratorReferenceType<I>>{};
        }
        template <typename I>
        constexpr auto TraitsCategory()
        {
            if constexpr (requires { typename I::IteratorCategory; })
                return TypeIdentity<typename I::IteratorCategory>{};
            else return TypeIdentity<IteratorConceptType<I>>{};
        }
    } // namespace Details

    template <typename Iterator>
    struct IteratorTraits
    {
      public:
        using DifferenceType =
            typename decltype(Details::TraitsDifference<Iterator>())::Type;
        using ValueType =
            typename decltype(Details::TraitsValue<Iterator>())::Type;
        using Pointer =
            typename decltype(Details::TraitsPointer<Iterator>())::Type;
        using Reference =
            typename decltype(Details::TraitsReference<Iterator>())::Type;
        using IteratorCategory =
            typename decltype(Details::TraitsCategory<Iterator>())::Type;
    };

    template <typename Iterator>
    struct ReverseIterator
    {
      public:
        using IteratorType     = Iterator;
        using IteratorCategory = ConditionalType<
            IsBaseOfV<RandomAccessIteratorTag,
                      typename IteratorTraits<Iterator>::IteratorCategory>,
            RandomAccessIteratorTag,
            typename IteratorTraits<Iterator>::IteratorCategory>;
        using ValueType = typename IteratorTraits<Iterator>::ValueType;
        using DifferenceType =
            typename IteratorTraits<Iterator>::DifferenceType;
        using Pointer   = typename IteratorTraits<Iterator>::Pointer;
        using Reference = typename IteratorTraits<Iterator>::Reference;

        constexpr ReverseIterator() = default;
        explicit constexpr ReverseIterator(IteratorType iter)
            : m_Current(iter)
        {
        }

        template <typename OtherIterator>
        constexpr ReverseIterator(const ReverseIterator<OtherIterator>& other)
            : m_Current(other.Base())
        {
        }

        constexpr IteratorType Base() const { return m_Current; }

        constexpr Reference    operator*() const
        {
            IteratorType tmp = m_Current;
            return *--tmp;
        }
        constexpr Pointer operator->() const { return AddressOf(operator*()); }

        constexpr ReverseIterator& operator++()
        {
            --m_Current;
            return *this;
        }
        constexpr ReverseIterator operator++(int)
        {
            ReverseIterator tmp = *this;
            --m_Current;
            return tmp;
        }
        constexpr ReverseIterator& operator--()
        {
            ++m_Current;
            return *this;
        }
        constexpr ReverseIterator operator--(int)
        {
            ReverseIterator tmp = *this;
            ++m_Current;
            return tmp;
        }

        constexpr ReverseIterator operator+(DifferenceType n) const
        {
            return ReverseIterator(m_Current - n);
        }
        constexpr ReverseIterator& operator+=(DifferenceType n)
        {
            m_Current -= n;
            return *this;
        }
        constexpr ReverseIterator operator-(DifferenceType n) const
        {
            return ReverseIterator(m_Current + n);
        }

        constexpr ReverseIterator& operator-=(DifferenceType n)
        {
            m_Current += n;
            return *this;
        }

        constexpr Reference operator[](DifferenceType n) const
        {
            return *(*this + n);
        }

        friend constexpr bool operator==(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current == rhs.m_Current;
        }
        friend constexpr bool operator!=(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current != rhs.m_Current;
        }

        friend constexpr bool operator<(const ReverseIterator& lhs,
                                        const ReverseIterator& rhs)
        {
            return lhs.m_Current > rhs.m_Current;
        }

        friend constexpr bool operator>(const ReverseIterator& lhs,
                                        const ReverseIterator& rhs)
        {
            return lhs.m_Current < rhs.m_Current;
        }

        friend constexpr bool operator<=(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current >= rhs.m_Current;
        }
        friend constexpr bool operator>=(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current <= rhs.m_Current;
        }

        friend constexpr DifferenceType operator-(const ReverseIterator& lhs,
                                                  const ReverseIterator& rhs)
        {
            return rhs.m_Current - lhs.m_Current;
        }
        friend constexpr ReverseIterator operator+(DifferenceType         n,
                                                   const ReverseIterator& it)
        {
            return it + n;
        }

      private:
        IteratorType m_Current;
    };

    template <typename It, typename Distance>
    constexpr void Advance(It& it, Distance n)
    {
        using Category = typename IteratorTraits<It>::IteratorCategory;
        static_assert(IsBaseOfV<InputIteratorTag, Category>);

        auto dist = typename IteratorTraits<It>::DifferenceType(n);
        if constexpr (IsBaseOfV<RandomAccessIteratorTag, Category>) it += dist;
        else {
            while (dist > 0)
            {
                --dist;
                ++it;
            }
            if constexpr (IsBaseOfV<BidirectionalIteratorTag, Category>)
                while (dist < 0)
                {
                    ++dist;
                    --it;
                }
        }
    }
    template <typename It>
    constexpr It Next(It it, typename IteratorTraits<It>::DifferenceType n = 1)
    {
        Advance(it, n);
        return it;
    }
    template <typename It>
    constexpr typename IteratorTraits<It>::DifferenceType Distance(It first,
                                                                   It last)
    {
        using Category = typename IteratorTraits<It>::IteratorCategory;
        static_assert(IsBaseOfV<InputIteratorTag, Category>);

        if constexpr (IsBaseOfV<RandomAccessIteratorTag, Category>)
            return last - first;
        else {
            typename IteratorTraits<It>::DifferenceType result = 0;
            while (first != last)
            {
                ++first;
                ++result;
            }
            return result;
        }
    }
    template <typename ForwardIt1, typename ForwardIt2>
    constexpr void IteratorSwap(ForwardIt1 a, ForwardIt2 b)
    {
        Swap(*a, *b);
    }

    namespace Ranges
    {
        // Iterator operations (std::ranges::advance, next, prev, distance)

        template <InputOrOutputIterator I>
        constexpr void Advance(I& it, IteratorDifferenceType<I> n)
        {
            if constexpr (RandomAccessIterator<I>) it += n;
            else
            {
                while (n > 0)
                {
                    --n;
                    ++it;
                }
                if constexpr (BidirectionalIterator<I>)
                    while (n < 0)
                    {
                        ++n;
                        --it;
                    }
            }
        }
        template <InputOrOutputIterator I, SentinelFor<I> S>
        constexpr void Advance(I& it, S bound)
        {
            if constexpr (AssignableFrom<I&, S>) it = Move(bound);
            else if constexpr (SizedSentinelFor<S, I>)
                Ranges::Advance(it, bound - it);
            else
                while (it != bound) ++it;
        }
        /// Advances by n, but not past bound; returns the steps not taken.
        template <InputOrOutputIterator I, SentinelFor<I> S>
        constexpr IteratorDifferenceType<I>
        Advance(I& it, IteratorDifferenceType<I> n, S bound)
        {
            if constexpr (SizedSentinelFor<S, I>)
            {
                const IteratorDifferenceType<I> distance = bound - it;
                if (n >= 0 ? distance <= n : distance >= n)
                {
                    Ranges::Advance(it, bound);
                    return n - distance;
                }
                Ranges::Advance(it, n);
                return 0;
            }
            else
            {
                while (n > 0 && it != bound)
                {
                    ++it;
                    --n;
                }
                if constexpr (BidirectionalIterator<I> && SameAs<I, S>)
                    while (n < 0 && it != bound)
                    {
                        --it;
                        ++n;
                    }
                return n;
            }
        }

        template <InputOrOutputIterator I>
        constexpr I Next(I it)
        {
            ++it;
            return it;
        }
        template <InputOrOutputIterator I>
        constexpr I Next(I it, IteratorDifferenceType<I> n)
        {
            Ranges::Advance(it, n);
            return it;
        }
        template <InputOrOutputIterator I, SentinelFor<I> S>
        constexpr I Next(I it, S bound)
        {
            Ranges::Advance(it, bound);
            return it;
        }
        template <InputOrOutputIterator I, SentinelFor<I> S>
        constexpr I Next(I it, IteratorDifferenceType<I> n, S bound)
        {
            Ranges::Advance(it, n, bound);
            return it;
        }

        template <BidirectionalIterator I>
        constexpr I Prev(I it)
        {
            --it;
            return it;
        }
        template <BidirectionalIterator I>
        constexpr I Prev(I it, IteratorDifferenceType<I> n)
        {
            Ranges::Advance(it, -n);
            return it;
        }
        template <BidirectionalIterator I>
        constexpr I Prev(I it, IteratorDifferenceType<I> n, I bound)
        {
            Ranges::Advance(it, -n, bound);
            return it;
        }

        template <InputOrOutputIterator I, SentinelFor<I> S>
        constexpr IteratorDifferenceType<I> Distance(I first, S last)
        {
            if constexpr (SizedSentinelFor<S, I>) return last - first;
            else
            {
                IteratorDifferenceType<I> count = 0;
                for (; first != last; ++first) ++count;
                return count;
            }
        }
    } // namespace Ranges
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::Advance;
using Prism::BidirectionalIteratorTag;
using Prism::ContiguousIteratorTag;
using Prism::DefaultSentinelType;
using Prism::Distance;
using Prism::ForwardIteratorTag;
using Prism::InputIteratorTag;
using Prism::IterationResult;
using Prism::IteratorReferenceType;
using Prism::IteratorSwap;
using Prism::IteratorTraits;
using Prism::Next;
using Prism::OutputIteratorTag;
using Prism::RandomAccessIteratorTag;
using Prism::ReverseIterator;
#endif
