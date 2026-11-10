/*
 * Created by v1tr10l7 on 09.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Iterator.hpp>

/**
 * Prism's counterpart of the standard <ranges> core: range access
 * (Ranges::Begin, End, Size, Data, ...), range concepts (Ranges::Range,
 * ForwardRange, ContiguousRange, View, ...), ViewInterface, Subrange and
 * Dangling. The views themselves live in <Prism/Ranges/Views.hpp>.
 *
 * Range access accepts both Prism member names (Size(), Empty(), Data(),
 * Raw()) and the standard library's (size(), empty(), data()), so Prism and
 * std containers are ranges alike. Iteration always uses begin()/end(),
 * like range-for.
 */
namespace Prism
{
    namespace Ranges
    {
        /// Base class of all views (std::ranges::view_base).
        struct ViewBase
#if PRISM_STD_INTEROP != 0
            : std::ranges::view_base
#endif
        {
        };

        /**
         * Specialize to true for ranges whose iterators stay valid after the
         * range itself is gone (std::ranges::enable_borrowed_range).
         */
        template <typename T>
        inline constexpr bool EnableBorrowedRange =
#if PRISM_STD_INTEROP != 0
            std::ranges::enable_borrowed_range<T>;
#else
            false;
#endif

        /// Specialize to true for views that don't derive from ViewBase.
        template <typename T>
        inline constexpr bool EnableView = DerivedFrom<T, ViewBase>
#if PRISM_STD_INTEROP != 0
                                        || std::ranges::enable_view<T>
#endif
            ;

        namespace Impl
        {
            template <typename T>
            constexpr DecayType<T> DecayCopy(T&& value)
            {
                return Forward<T>(value);
            }

            template <typename T>
            concept MaybeBorrowedRange
                = IsLValueReferenceV<T>
               || EnableBorrowedRange<RemoveCvRefType<T>>;

            template <typename T>
            using AsConstType
                = ConditionalType<IsLValueReferenceV<T>,
                                  const RemoveReferenceType<T>&,
                                  const RemoveReferenceType<T>&&>;

            // Poison pills: unqualified calls only find these or ADL.
            void begin(auto&)       = delete;
            void begin(const auto&) = delete;
            void end(auto&)         = delete;
            void end(const auto&)   = delete;
            void size(auto&)        = delete;
            void size(const auto&)  = delete;

            // Begin
            template <typename T>
            concept MemberBegin = requires(T& t) {
                { DecayCopy(t.begin()) } -> InputOrOutputIterator;
            };
            template <typename T>
            concept AdlBegin = ClassOrEnum<RemoveReferenceType<T>>
                            && requires(T& t) {
                                   { DecayCopy(begin(t)) } -> InputOrOutputIterator;
                               };

            struct BeginFn
            {
                template <typename T>
                    requires MaybeBorrowedRange<T>
                          && (IsArrayV<RemoveReferenceType<T>> || MemberBegin<T>
                              || AdlBegin<T>)
                constexpr auto operator()(T&& t) const
                {
                    if constexpr (IsArrayV<RemoveReferenceType<T>>) return t + 0;
                    else if constexpr (MemberBegin<T>) return t.begin();
                    else return begin(t);
                }
            };
            template <typename T>
            using BeginType = decltype(BeginFn{}(DeclVal<T&>()));

            // End
            template <typename T>
            concept MemberEnd = requires(T& t) {
                { DecayCopy(t.end()) } -> SentinelFor<BeginType<T>>;
            };
            template <typename T>
            concept AdlEnd = ClassOrEnum<RemoveReferenceType<T>>
                          && requires(T& t) {
                                 { DecayCopy(end(t)) } -> SentinelFor<BeginType<T>>;
                             };

            struct EndFn
            {
                template <typename T>
                    requires MaybeBorrowedRange<T>
                          && (IsBoundedArrayV<RemoveReferenceType<T>>
                              || MemberEnd<T> || AdlEnd<T>)
                constexpr auto operator()(T&& t) const
                {
                    if constexpr (IsBoundedArrayV<RemoveReferenceType<T>>)
                        return t + (sizeof(t) / sizeof(t[0]));
                    else if constexpr (MemberEnd<T>) return t.end();
                    else return end(t);
                }
            };

            struct CBeginFn
            {
                template <typename T>
                    requires requires(T&& t) {
                        BeginFn{}(static_cast<AsConstType<T>>(t));
                    }
                constexpr auto operator()(T&& t) const
                {
                    return BeginFn{}(static_cast<AsConstType<T>>(t));
                }
            };
            struct CEndFn
            {
                template <typename T>
                    requires requires(T&& t) {
                        EndFn{}(static_cast<AsConstType<T>>(t));
                    }
                constexpr auto operator()(T&& t) const
                {
                    return EndFn{}(static_cast<AsConstType<T>>(t));
                }
            };

            // RBegin / REnd
            template <typename T>
            concept MemberRBegin = requires(T& t) {
                { DecayCopy(t.rbegin()) } -> InputOrOutputIterator;
            };
            template <typename T>
            concept ReversibleRange = requires(T& t) {
                { BeginFn{}(t) } -> BidirectionalIterator;
                { EndFn{}(t) } -> SameAs<BeginType<T>>;
            };

            struct RBeginFn
            {
                template <typename T>
                    requires MaybeBorrowedRange<T>
                          && (MemberRBegin<T> || ReversibleRange<T>)
                constexpr auto operator()(T&& t) const
                {
                    if constexpr (MemberRBegin<T>) return t.rbegin();
                    else
                        return ReverseIterator<BeginType<T>>(EndFn{}(t));
                }
            };
            template <typename T>
            using RBeginType = decltype(RBeginFn{}(DeclVal<T&>()));

            template <typename T>
            concept MemberREnd = requires(T& t) {
                { DecayCopy(t.rend()) } -> SentinelFor<RBeginType<T>>;
            };

            struct REndFn
            {
                template <typename T>
                    requires MaybeBorrowedRange<T>
                          && (MemberREnd<T> || ReversibleRange<T>)
                constexpr auto operator()(T&& t) const
                {
                    if constexpr (MemberREnd<T>) return t.rend();
                    else
                        return ReverseIterator<BeginType<T>>(BeginFn{}(t));
                }
            };

            struct CRBeginFn
            {
                template <typename T>
                    requires requires(T&& t) {
                        RBeginFn{}(static_cast<AsConstType<T>>(t));
                    }
                constexpr auto operator()(T&& t) const
                {
                    return RBeginFn{}(static_cast<AsConstType<T>>(t));
                }
            };
            struct CREndFn
            {
                template <typename T>
                    requires requires(T&& t) {
                        REndFn{}(static_cast<AsConstType<T>>(t));
                    }
                constexpr auto operator()(T&& t) const
                {
                    return REndFn{}(static_cast<AsConstType<T>>(t));
                }
            };

            // Size
            template <typename T>
            concept MemberPrismSize = requires(T& t) {
                { DecayCopy(t.Size()) } -> Integral;
            };
            template <typename T>
            concept MemberSize = requires(T& t) {
                { DecayCopy(t.size()) } -> Integral;
            };
            template <typename T>
            concept AdlSize = ClassOrEnum<RemoveReferenceType<T>>
                           && requires(T& t) {
                                  { DecayCopy(size(t)) } -> Integral;
                              };
            template <typename T>
            concept SizeFromIterators = requires(T& t) {
                { BeginFn{}(t) } -> ForwardIterator;
                { EndFn{}(t) } -> SizedSentinelFor<BeginType<T>>;
            };

            struct SizeFn
            {
                template <typename T>
                    requires IsBoundedArrayV<RemoveReferenceType<T>>
                          || MemberPrismSize<T> || MemberSize<T> || AdlSize<T>
                          || SizeFromIterators<T>
                constexpr auto operator()(T&& t) const
                {
                    if constexpr (IsBoundedArrayV<RemoveReferenceType<T>>)
                        return sizeof(t) / sizeof(t[0]);
                    else if constexpr (MemberPrismSize<T>) return t.Size();
                    else if constexpr (MemberSize<T>) return t.size();
                    else if constexpr (AdlSize<T>) return size(t);
                    else
                    {
                        using Difference
                            = decltype(EndFn{}(t) - BeginFn{}(t));
                        return static_cast<MakeUnsignedType<Difference>>(
                            EndFn{}(t) - BeginFn{}(t));
                    }
                }
            };

            struct SSizeFn
            {
                template <typename T>
                    requires requires(T&& t) { SizeFn{}(t); }
                constexpr auto operator()(T&& t) const
                {
                    using Signed = MakeSignedType<decltype(SizeFn{}(t))>;
                    using Result = ConditionalType<(sizeof(Signed)
                                                    < sizeof(ptrdiff)),
                                                   ptrdiff, Signed>;
                    return static_cast<Result>(SizeFn{}(t));
                }
            };

            // Empty
            template <typename T>
            concept MemberPrismEmpty
                = requires(T& t) { static_cast<bool>(t.Empty()); };
            template <typename T>
            concept MemberEmpty
                = requires(T& t) { static_cast<bool>(t.empty()); };
            template <typename T>
            concept SizeEmpty = requires(T& t) { SizeFn{}(t) == 0; };
            template <typename T>
            concept IteratorsEmpty = requires(T& t) {
                { BeginFn{}(t) } -> ForwardIterator;
                static_cast<bool>(BeginFn{}(t) == EndFn{}(t));
            };

            struct EmptyFn
            {
                template <typename T>
                    requires MemberPrismEmpty<T> || MemberEmpty<T>
                          || SizeEmpty<T> || IteratorsEmpty<T>
                constexpr bool operator()(T&& t) const
                {
                    if constexpr (MemberPrismEmpty<T>)
                        return static_cast<bool>(t.Empty());
                    else if constexpr (MemberEmpty<T>)
                        return static_cast<bool>(t.empty());
                    else if constexpr (SizeEmpty<T>) return SizeFn{}(t) == 0;
                    else return static_cast<bool>(BeginFn{}(t) == EndFn{}(t));
                }
            };

            // Data
            template <typename T>
            concept PointerToObject
                = IsPointerV<T> && IsObjectV<RemovePointerType<T>>;
            template <typename T>
            concept MemberData = requires(T& t) {
                { DecayCopy(t.data()) } -> PointerToObject;
            };
            template <typename T>
            concept MemberPrismData = requires(T& t) {
                { DecayCopy(t.Data()) } -> PointerToObject;
            };
            template <typename T>
            concept MemberRaw = requires(T& t) {
                { DecayCopy(t.Raw()) } -> PointerToObject;
            };
            template <typename T>
            concept ContiguousBegin = requires(T& t) {
                { BeginFn{}(t) } -> ContiguousIterator;
            };

            struct DataFn
            {
                template <typename T>
                    requires MaybeBorrowedRange<T>
                          && (MemberData<T> || MemberPrismData<T>
                              || MemberRaw<T> || ContiguousBegin<T>)
                constexpr auto operator()(T&& t) const
                {
                    if constexpr (MemberData<T>) return t.data();
                    else if constexpr (MemberPrismData<T>) return t.Data();
                    else if constexpr (MemberRaw<T>) return t.Raw();
                    else return ::Prism::Details::ToAddressOf(BeginFn{}(t));
                }
            };
            struct CDataFn
            {
                template <typename T>
                    requires requires(T&& t) {
                        DataFn{}(static_cast<AsConstType<T>>(t));
                    }
                constexpr auto operator()(T&& t) const
                {
                    return DataFn{}(static_cast<AsConstType<T>>(t));
                }
            };
        } // namespace Impl

        // Range access (std::ranges::begin, ...)
        inline constexpr Impl::BeginFn   Begin{};
        inline constexpr Impl::EndFn     End{};
        inline constexpr Impl::CBeginFn  CBegin{};
        inline constexpr Impl::CEndFn    CEnd{};
        inline constexpr Impl::RBeginFn  RBegin{};
        inline constexpr Impl::REndFn    REnd{};
        inline constexpr Impl::CRBeginFn CRBegin{};
        inline constexpr Impl::CREndFn   CREnd{};
        inline constexpr Impl::SizeFn    Size{};
        inline constexpr Impl::SSizeFn   SSize{};
        inline constexpr Impl::EmptyFn   Empty{};
        inline constexpr Impl::DataFn    Data{};
        inline constexpr Impl::CDataFn   CData{};

        // ======================================================================
        // Range concepts and associated types
        template <typename T>
        concept Range = requires(T& t) {
            Ranges::Begin(t);
            Ranges::End(t);
        };
        template <typename T>
        concept BorrowedRange = Range<T> && Impl::MaybeBorrowedRange<T>;

        template <Range R>
        using IteratorType = decltype(Ranges::Begin(DeclVal<R&>()));
        template <Range R>
        using SentinelType = decltype(Ranges::End(DeclVal<R&>()));
        template <Range R>
        using ConstIteratorType = decltype(Ranges::CBegin(DeclVal<R&>()));
        template <Range R>
        using ConstSentinelType = decltype(Ranges::CEnd(DeclVal<R&>()));
        template <Range R>
        using RangeDifferenceType = IteratorDifferenceType<IteratorType<R>>;
        template <Range R>
        using RangeValueType = IteratorValueType<IteratorType<R>>;
        template <Range R>
        using RangeReferenceType = IteratorReferenceType<IteratorType<R>>;
        template <Range R>
        using RangeRValueReferenceType
            = IteratorRValueReferenceType<IteratorType<R>>;
        template <Range R>
        using RangeCommonReferenceType
            = IteratorCommonReferenceType<IteratorType<R>>;

        template <typename T>
        concept SizedRange = Range<T> && requires(T& t) { Ranges::Size(t); };
        template <SizedRange R>
        using RangeSizeType = decltype(Ranges::Size(DeclVal<R&>()));

        template <typename T>
        concept View = Range<T> && Movable<T> && EnableView<T>;

        template <typename R, typename T>
        concept OutputRange = Range<R> && OutputIterator<IteratorType<R>, T>;
        template <typename R>
        concept InputRange = Range<R> && InputIterator<IteratorType<R>>;
        template <typename R>
        concept ForwardRange
            = InputRange<R> && ForwardIterator<IteratorType<R>>;
        template <typename R>
        concept BidirectionalRange
            = ForwardRange<R> && BidirectionalIterator<IteratorType<R>>;
        template <typename R>
        concept RandomAccessRange
            = BidirectionalRange<R> && RandomAccessIterator<IteratorType<R>>;
        template <typename R>
        concept ContiguousRange
            = RandomAccessRange<R> && ContiguousIterator<IteratorType<R>>
           && requires(R& r) {
                  {
                      Ranges::Data(r)
                  } -> SameAs<AddPointerType<RangeReferenceType<R>>>;
              };
        template <typename R>
        concept CommonRange
            = Range<R> && SameAs<IteratorType<R>, SentinelType<R>>;

        namespace Impl
        {
            template <typename T>
            inline constexpr bool IsInitializerList = false;
            template <typename T>
            inline constexpr bool IsInitializerList<InitializerList<T>> = true;
        } // namespace Impl

        template <typename T>
        concept ViewableRange
            = Range<T>
           && ((View<RemoveCvRefType<T>>
                && ConstructibleFrom<RemoveCvRefType<T>, T>)
               || (!View<RemoveCvRefType<T>>
                   && (IsLValueReferenceV<T>
                       || (Movable<RemoveReferenceType<T>>
                           && !Impl::IsInitializerList<RemoveCvRefType<T>>))));

        /// std::ranges::distance for a whole range.
        template <Range R>
        constexpr RangeDifferenceType<R> Distance(R&& range)
        {
            if constexpr (SizedRange<R>)
                return static_cast<RangeDifferenceType<R>>(Ranges::Size(range));
            else return Ranges::Distance(Ranges::Begin(range), Ranges::End(range));
        }

        /// Returned instead of an iterator into a range that is gone.
        struct Dangling
        {
            constexpr Dangling() PM_NOEXCEPT = default;
            template <typename... Args>
            constexpr Dangling(Args&&...) PM_NOEXCEPT
            {
            }
        };
        template <Range R>
        using BorrowedIteratorType
            = ConditionalType<BorrowedRange<R>, IteratorType<R>, Dangling>;

        // ======================================================================
        /**
         * CRTP base for views (std::ranges::view_interface): derives Empty(),
         * explicit operator bool, Data(), Size(), Front(), Back() and
         * operator[] from the view's begin() and end().
         */
        template <typename D>
            requires IsClassV<D> && SameAs<D, RemoveCvType<D>>
        class ViewInterface : public ViewBase
        {
            constexpr D&       Derived() { return static_cast<D&>(*this); }
            constexpr const D& Derived() const
            {
                return static_cast<const D&>(*this);
            }

          public:
            constexpr bool Empty()
                requires SizedRange<D> || ForwardRange<D>
            {
                if constexpr (SizedRange<D>) return Ranges::Size(Derived()) == 0;
                else return Ranges::Begin(Derived()) == Ranges::End(Derived());
            }
            constexpr bool Empty() const
                requires SizedRange<const D> || ForwardRange<const D>
            {
                if constexpr (SizedRange<const D>)
                    return Ranges::Size(Derived()) == 0;
                else return Ranges::Begin(Derived()) == Ranges::End(Derived());
            }

            constexpr explicit operator bool()
                requires requires(D& d) { Ranges::Empty(d); }
            {
                return !Ranges::Empty(Derived());
            }
            constexpr explicit operator bool() const
                requires requires(const D& d) { Ranges::Empty(d); }
            {
                return !Ranges::Empty(Derived());
            }

            template <typename R = D>
                requires ContiguousIterator<IteratorType<R>>
            constexpr auto Data()
            {
                return ::Prism::Details::ToAddressOf(Ranges::Begin(Derived()));
            }
            template <typename R = const D>
                requires Range<R> && ContiguousIterator<IteratorType<R>>
            constexpr auto Data() const
            {
                return ::Prism::Details::ToAddressOf(Ranges::Begin(Derived()));
            }

            template <typename R = D>
                requires ForwardRange<R>
                      && SizedSentinelFor<SentinelType<R>, IteratorType<R>>
            constexpr auto Size()
            {
                return static_cast<MakeUnsignedType<RangeDifferenceType<R>>>(
                    Ranges::End(Derived()) - Ranges::Begin(Derived()));
            }
            template <typename R = const D>
                requires ForwardRange<R>
                      && SizedSentinelFor<SentinelType<R>, IteratorType<R>>
            constexpr auto Size() const
            {
                return static_cast<MakeUnsignedType<RangeDifferenceType<R>>>(
                    Ranges::End(Derived()) - Ranges::Begin(Derived()));
            }

            template <typename R = D>
                requires ForwardRange<R>
            constexpr decltype(auto) Front()
            {
                return *Ranges::Begin(Derived());
            }
            template <typename R = const D>
                requires ForwardRange<R>
            constexpr decltype(auto) Front() const
            {
                return *Ranges::Begin(Derived());
            }

            template <typename R = D>
                requires BidirectionalRange<R> && CommonRange<R>
            constexpr decltype(auto) Back()
            {
                return *Ranges::Prev(Ranges::End(Derived()));
            }
            template <typename R = const D>
                requires BidirectionalRange<R> && CommonRange<R>
            constexpr decltype(auto) Back() const
            {
                return *Ranges::Prev(Ranges::End(Derived()));
            }

            template <RandomAccessRange R = D>
            constexpr decltype(auto) operator[](RangeDifferenceType<R> n)
            {
                return Ranges::Begin(Derived())[n];
            }
            template <RandomAccessRange R = const D>
            constexpr decltype(auto) operator[](RangeDifferenceType<R> n) const
            {
                return Ranges::Begin(Derived())[n];
            }
        };

        // ======================================================================
        /// An iterator-sentinel pair as a view (std::ranges::subrange).
        template <InputOrOutputIterator I, SentinelFor<I> S = I>
        class Subrange : public ViewInterface<Subrange<I, S>>
        {
          public:
            constexpr Subrange()
                requires DefaultInitializable<I>
            = default;
            constexpr Subrange(I begin, S end)
                : m_Begin(Move(begin))
                , m_End(Move(end))
            {
            }
            template <BorrowedRange R>
                requires(!SameAs<RemoveCvRefType<R>, Subrange>)
                     && ConvertibleTo<IteratorType<R>, I>
                     && ConvertibleTo<SentinelType<R>, S>
            constexpr Subrange(R&& range)
                : Subrange(Ranges::Begin(range), Ranges::End(range))
            {
            }

            constexpr I begin() const
                requires Copyable<I>
            {
                return m_Begin;
            }
            constexpr I begin()
                requires(!Copyable<I>)
            {
                return Move(m_Begin);
            }
            constexpr S    end() const { return m_End; }

            constexpr bool Empty() const { return m_Begin == m_End; }

            constexpr Subrange& Advance(IteratorDifferenceType<I> n)
            {
                if constexpr (BidirectionalIterator<I>)
                    if (n < 0)
                    {
                        Ranges::Advance(m_Begin, n);
                        return *this;
                    }
                Ranges::Advance(m_Begin, n, m_End);
                return *this;
            }
            constexpr Subrange Next(IteratorDifferenceType<I> n = 1) const&
                requires ForwardIterator<I>
            {
                Subrange copy = *this;
                copy.Advance(n);
                return copy;
            }
            constexpr Subrange Next(IteratorDifferenceType<I> n = 1) &&
            {
                Advance(n);
                return Move(*this);
            }
            constexpr Subrange Prev(IteratorDifferenceType<I> n = 1) const
                requires BidirectionalIterator<I>
            {
                Subrange copy = *this;
                copy.Advance(-n);
                return copy;
            }

          private:
            I m_Begin{};
            S m_End{};
        };

        template <InputOrOutputIterator I, SentinelFor<I> S>
        Subrange(I, S) -> Subrange<I, S>;
        template <BorrowedRange R>
        Subrange(R&&) -> Subrange<IteratorType<R>, SentinelType<R>>;

        template <typename I, typename S>
        inline constexpr bool EnableBorrowedRange<Subrange<I, S>> = true;

        template <Range R>
        using BorrowedSubrangeType
            = ConditionalType<BorrowedRange<R>, Subrange<IteratorType<R>>,
                              Dangling>;
    } // namespace Ranges
}; // namespace Prism
