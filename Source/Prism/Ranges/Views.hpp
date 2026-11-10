/*
 * Created by v1tr10l7 on 09.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Ranges/Range.hpp>

/**
 * Prism's range views and adaptors (std::views / std::ranges::*_view):
 *
 *   Views::All, Views::Empty<T>, Views::Single, Views::Iota, Views::Counted,
 *   Views::Filter, Views::Transform, Views::Take, Views::TakeWhile,
 *   Views::Drop, Views::DropWhile, Views::Reverse, Views::Join,
 *   Views::Elements<N>, Views::Keys, Views::Values, Views::Enumerate,
 *   and Ranges::To<Container>.
 *
 * Adaptors compose with `|`:
 *
 *   for (auto x : vec | Views::Filter(isOdd) | Views::Transform(square))
 *
 * Differences from std: views don't cache begin() (FilterView, DropView and
 * DropWhileView compute it on every call), Views::Join needs inner ranges
 * that are references with default-constructible iterators, and
 * Views::Enumerate yields an aggregate with Index and Value members.
 */
namespace Prism
{
    namespace Views
    {
    }

    namespace Ranges
    {
        namespace Views = ::Prism::Views;

        // ======================================================================
        // Adaptor machinery

        /**
         * Base class of range adaptor closures: an object C such that
         * `range | C` is `C(range)` (std::ranges::range_adaptor_closure).
         */
        template <typename D>
        struct RangeAdaptorClosure
        {
        };

        namespace Impl
        {
            template <typename T>
            concept AdaptorClosure
                = IsClassV<RemoveCvRefType<T>>
               && DerivedFrom<RemoveCvRefType<T>,
                              RangeAdaptorClosure<RemoveCvRefType<T>>>;

            /// `left | right` for two closures.
            template <typename L, typename R>
            struct Pipe : RangeAdaptorClosure<Pipe<L, R>>
            {
                L m_Left;
                R m_Right;

                constexpr Pipe(L left, R right)
                    : m_Left(Move(left))
                    , m_Right(Move(right))
                {
                }

                template <typename T>
                    requires Invocable<const L&, T>
                          && Invocable<const R&, InvokeResultType<const L&, T>>
                constexpr auto operator()(T&& t) const&
                {
                    return m_Right(m_Left(Forward<T>(t)));
                }
                template <typename T>
                    requires Invocable<L, T>
                          && Invocable<R, InvokeResultType<L, T>>
                constexpr auto operator()(T&& t) &&
                {
                    return Move(m_Right)(Move(m_Left)(Forward<T>(t)));
                }
            };

            /// An adaptor with its extra argument bound: `Adaptor{}(range, arg)`.
            template <typename Adaptor, typename Arg>
            struct Partial : RangeAdaptorClosure<Partial<Adaptor, Arg>>
            {
                Arg m_Arg;

                constexpr explicit Partial(Arg arg)
                    : m_Arg(Move(arg))
                {
                }

                template <typename R>
                    requires requires {
                        Adaptor{}(DeclVal<R>(), DeclVal<const Arg&>());
                    }
                constexpr auto operator()(R&& range) const&
                {
                    return Adaptor{}(Forward<R>(range), m_Arg);
                }
                template <typename R>
                    requires requires {
                        Adaptor{}(DeclVal<R>(), DeclVal<Arg>());
                    }
                constexpr auto operator()(R&& range) &&
                {
                    return Adaptor{}(Forward<R>(range), Move(m_Arg));
                }
            };

            /**
             * Holds a view's function object or value and keeps the view
             * assignable even when T isn't (e.g. a lambda), like std's
             * movable-box.
             */
            template <typename T>
                requires MoveConstructible<T> && IsObjectV<T>
            class MovableBox
            {
              public:
                constexpr MovableBox()
                    requires DefaultInitializable<T>
                    : m_Engaged(true)
                    , m_Value()
                {
                }
                template <typename... Args>
                    requires ConstructibleFrom<T, Args...>
                constexpr explicit MovableBox(InPlaceT, Args&&... args)
                    : m_Engaged(true)
                    , m_Value(Forward<Args>(args)...)
                {
                }
                constexpr MovableBox(const MovableBox& other)
                    requires CopyConstructible<T>
                    : m_Engaged(other.m_Engaged)
                {
                    if (m_Engaged) ConstructAt(AddressOf(m_Value), other.m_Value);
                }
                constexpr MovableBox(MovableBox&& other)
                    : m_Engaged(other.m_Engaged)
                {
                    if (m_Engaged)
                        ConstructAt(AddressOf(m_Value), Move(other.m_Value));
                }
                constexpr ~MovableBox() { Reset(); }

                constexpr MovableBox& operator=(const MovableBox& other)
                    requires CopyConstructible<T>
                {
                    if (this != AddressOf(other))
                    {
                        Reset();
                        if (other.m_Engaged)
                        {
                            ConstructAt(AddressOf(m_Value), other.m_Value);
                            m_Engaged = true;
                        }
                    }
                    return *this;
                }
                constexpr MovableBox& operator=(MovableBox&& other)
                {
                    if (this != AddressOf(other))
                    {
                        Reset();
                        if (other.m_Engaged)
                        {
                            ConstructAt(AddressOf(m_Value),
                                        Move(other.m_Value));
                            m_Engaged = true;
                        }
                    }
                    return *this;
                }

                constexpr T&       operator*() { return m_Value; }
                constexpr const T& operator*() const { return m_Value; }
                constexpr T*       operator->() { return AddressOf(m_Value); }
                constexpr const T* operator->() const
                {
                    return AddressOf(m_Value);
                }

              private:
                constexpr void Reset()
                {
                    if (!m_Engaged) return;
                    DestroyAt(AddressOf(m_Value));
                    m_Engaged = false;
                }

                bool m_Engaged = false;
                union
                {
                    T m_Value;
                };
            };

            template <bool Const, typename T>
            using MaybeConst = ConditionalType<Const, const T, T>;

            template <typename V>
            using IteratorConceptOf = ConditionalType<
                RandomAccessRange<V>, RandomAccessIteratorTag,
                ConditionalType<
                    BidirectionalRange<V>, BidirectionalIteratorTag,
                    ConditionalType<ForwardRange<V>, ForwardIteratorTag,
                                    InputIteratorTag>>>;
        } // namespace Impl

        template <typename R, typename C>
            requires Impl::AdaptorClosure<C> && (!Impl::AdaptorClosure<R>)
                  && Invocable<C, R>
        constexpr auto operator|(R&& range, C&& closure)
        {
            return Invoke(Forward<C>(closure), Forward<R>(range));
        }
        template <typename L, typename R>
            requires Impl::AdaptorClosure<L> && Impl::AdaptorClosure<R>
        constexpr auto operator|(L&& left, R&& right)
        {
            return Impl::Pipe<DecayType<L>, DecayType<R>>(Forward<L>(left),
                                                          Forward<R>(right));
        }

        // ======================================================================
        // RefView, OwningView, Views::All

        /// A view of a range it doesn't own (std::ranges::ref_view).
        template <Range R>
            requires IsObjectV<R>
        class RefView : public ViewInterface<RefView<R>>
        {
          public:
            constexpr RefView(R& range)
                : m_Range(AddressOf(range))
            {
            }

            constexpr R&              Base() const { return *m_Range; }

            constexpr IteratorType<R> begin() const
            {
                return Ranges::Begin(*m_Range);
            }
            constexpr SentinelType<R> end() const
            {
                return Ranges::End(*m_Range);
            }

            constexpr bool Empty() const
                requires requires(R& r) { Ranges::Empty(r); }
            {
                return Ranges::Empty(*m_Range);
            }
            constexpr auto Size() const
                requires SizedRange<R>
            {
                return Ranges::Size(*m_Range);
            }
            constexpr auto Data() const
                requires ContiguousRange<R>
            {
                return Ranges::Data(*m_Range);
            }

          private:
            R* m_Range;
        };
        template <typename R>
        RefView(R&) -> RefView<R>;
        template <typename T>
        inline constexpr bool EnableBorrowedRange<RefView<T>> = true;

        /// A move-only view that owns an rvalue range (std::ranges::owning_view).
        template <Range R>
            requires Movable<R> && (!Impl::IsInitializerList<RemoveCvType<R>>)
        class OwningView : public ViewInterface<OwningView<R>>
        {
          public:
            constexpr OwningView()
                requires DefaultInitializable<R>
            = default;
            constexpr OwningView(R&& range)
                : m_Range(Move(range))
            {
            }
            constexpr OwningView(OwningView&&)                 = default;
            constexpr OwningView& operator=(OwningView&&)      = default;
            OwningView(const OwningView&)                      = delete;
            OwningView&         operator=(const OwningView&)   = delete;

            constexpr R&        Base() & { return m_Range; }
            constexpr const R&  Base() const& { return m_Range; }
            constexpr R&&       Base() && { return Move(m_Range); }

            constexpr auto      begin() { return Ranges::Begin(m_Range); }
            constexpr auto      end() { return Ranges::End(m_Range); }
            constexpr auto      begin() const
                requires Range<const R>
            {
                return Ranges::Begin(m_Range);
            }
            constexpr auto end() const
                requires Range<const R>
            {
                return Ranges::End(m_Range);
            }

            constexpr bool Empty()
                requires requires(R& r) { Ranges::Empty(r); }
            {
                return Ranges::Empty(m_Range);
            }
            constexpr bool Empty() const
                requires requires(const R& r) { Ranges::Empty(r); }
            {
                return Ranges::Empty(m_Range);
            }
            constexpr auto Size()
                requires SizedRange<R>
            {
                return Ranges::Size(m_Range);
            }
            constexpr auto Size() const
                requires SizedRange<const R>
            {
                return Ranges::Size(m_Range);
            }
            constexpr auto Data()
                requires ContiguousRange<R>
            {
                return Ranges::Data(m_Range);
            }
            constexpr auto Data() const
                requires ContiguousRange<const R>
            {
                return Ranges::Data(m_Range);
            }

          private:
            R m_Range{};
        };
        template <typename T>
        inline constexpr bool EnableBorrowedRange<OwningView<T>>
            = EnableBorrowedRange<T>;

        namespace Impl
        {
            struct AllFn : RangeAdaptorClosure<AllFn>
            {
                template <ViewableRange R>
                constexpr auto operator()(R&& range) const
                {
                    if constexpr (View<DecayType<R>>)
                        return DecayType<R>(Forward<R>(range));
                    else if constexpr (IsLValueReferenceV<R>)
                        return RefView<RemoveReferenceType<R>>(range);
                    else
                        return OwningView<RemoveReferenceType<R>>(
                            Move(range));
                }
            };
        } // namespace Impl
    } // namespace Ranges

    namespace Views
    {
        /// The range itself if it's a view, else a RefView or OwningView of it.
        inline constexpr Ranges::Impl::AllFn All{};
    } // namespace Views

    namespace Ranges
    {
        template <ViewableRange R>
        using AllType = decltype(Views::All(DeclVal<R>()));

        // ======================================================================
        // EmptyView, SingleView

        template <typename T>
            requires IsObjectV<T>
        class EmptyView : public ViewInterface<EmptyView<T>>
        {
          public:
            static constexpr T*    begin() PM_NOEXCEPT { return nullptr; }
            static constexpr T*    end() PM_NOEXCEPT { return nullptr; }
            static constexpr T*    Data() PM_NOEXCEPT { return nullptr; }
            static constexpr usize Size() PM_NOEXCEPT { return 0; }
            static constexpr bool  Empty() PM_NOEXCEPT { return true; }
        };
        template <typename T>
        inline constexpr bool EnableBorrowedRange<EmptyView<T>> = true;

        template <typename T>
            requires MoveConstructible<T> && IsObjectV<T>
        class SingleView : public ViewInterface<SingleView<T>>
        {
          public:
            constexpr SingleView()
                requires DefaultInitializable<T>
            = default;
            constexpr explicit SingleView(const T& value)
                requires CopyConstructible<T>
                : m_Value(InPlace, value)
            {
            }
            constexpr explicit SingleView(T&& value)
                : m_Value(InPlace, Move(value))
            {
            }
            template <typename... Args>
                requires ConstructibleFrom<T, Args...>
            constexpr explicit SingleView(InPlaceT, Args&&... args)
                : m_Value(InPlace, Forward<Args>(args)...)
            {
            }

            constexpr T*           begin() { return Data(); }
            constexpr const T*     begin() const { return Data(); }
            constexpr T*           end() { return Data() + 1; }
            constexpr const T*     end() const { return Data() + 1; }

            static constexpr usize Size() PM_NOEXCEPT { return 1; }
            static constexpr bool  Empty() PM_NOEXCEPT { return false; }
            constexpr T*           Data() { return AddressOf(*m_Value); }
            constexpr const T*     Data() const { return AddressOf(*m_Value); }

          private:
            Impl::MovableBox<T> m_Value;
        };
        template <typename T>
        SingleView(T) -> SingleView<T>;

        namespace Impl
        {
            struct SingleFn
            {
                template <typename T>
                    requires requires {
                        SingleView<DecayType<T>>(DeclVal<T>());
                    }
                constexpr auto operator()(T&& value) const
                {
                    return SingleView<DecayType<T>>(Forward<T>(value));
                }
            };
        } // namespace Impl

        // ======================================================================
        // IotaView

        namespace Impl
        {
            template <typename W>
            using IotaDifferenceType = ConditionalType<
                !Integral<W>, IteratorDifferenceType<W>,
                ConditionalType<(sizeof(W) < sizeof(i32)), i32, i64>>;

            template <typename W>
            concept Decrementable = Incrementable<W> && requires(W i) {
                { --i } -> SameAs<W&>;
                { i-- } -> SameAs<W>;
            };
            template <typename W>
            concept Advanceable
                = Decrementable<W> && TotallyOrdered<W>
               && requires(W i, const W j, const IotaDifferenceType<W> n) {
                      { i += n } -> SameAs<W&>;
                      { i -= n } -> SameAs<W&>;
                      W(j + n);
                      W(n + j);
                      W(j - n);
                      { j - j } -> ConvertibleTo<IotaDifferenceType<W>>;
                  };
        } // namespace Impl

        /// value, value + 1, ... up to (not including) bound (std::ranges::iota_view).
        template <WeaklyIncrementable W,
                  SemiRegular         Bound = UnreachableSentinelType>
            requires ::Prism::Details::WeaklyEqualityComparableWith<W, Bound>
                  && Copyable<W>
        class IotaView : public ViewInterface<IotaView<W, Bound>>
        {
          public:
            class Iterator
            {
              public:
                using IteratorConcept = ConditionalType<
                    Impl::Advanceable<W>, RandomAccessIteratorTag,
                    ConditionalType<
                        Impl::Decrementable<W>, BidirectionalIteratorTag,
                        ConditionalType<Incrementable<W>, ForwardIteratorTag,
                                        InputIteratorTag>>>;
                using ValueType      = W;
                using DifferenceType = Impl::IotaDifferenceType<W>;

                Iterator()
                    requires DefaultInitializable<W>
                = default;
                constexpr explicit Iterator(W value)
                    : m_Value(value)
                {
                }

                constexpr const W& Value() const { return m_Value; }
                constexpr W        operator*() const { return m_Value; }

                constexpr Iterator& operator++()
                {
                    ++m_Value;
                    return *this;
                }
                constexpr void operator++(int)
                    requires(!Incrementable<W>)
                {
                    ++*this;
                }
                constexpr Iterator operator++(int)
                    requires Incrementable<W>
                {
                    Iterator copy = *this;
                    ++*this;
                    return copy;
                }
                constexpr Iterator& operator--()
                    requires Impl::Decrementable<W>
                {
                    --m_Value;
                    return *this;
                }
                constexpr Iterator operator--(int)
                    requires Impl::Decrementable<W>
                {
                    Iterator copy = *this;
                    --*this;
                    return copy;
                }
                constexpr Iterator& operator+=(DifferenceType n)
                    requires Impl::Advanceable<W>
                {
                    if constexpr (Integral<W>)
                    {
                        if (n >= DifferenceType(0))
                            m_Value += static_cast<W>(n);
                        else m_Value -= static_cast<W>(-n);
                    }
                    else m_Value += n;
                    return *this;
                }
                constexpr Iterator& operator-=(DifferenceType n)
                    requires Impl::Advanceable<W>
                {
                    if constexpr (Integral<W>)
                    {
                        if (n >= DifferenceType(0))
                            m_Value -= static_cast<W>(n);
                        else m_Value += static_cast<W>(-n);
                    }
                    else m_Value -= n;
                    return *this;
                }
                constexpr W operator[](DifferenceType n) const
                    requires Impl::Advanceable<W>
                {
                    Iterator copy = *this;
                    copy += n;
                    return copy.m_Value;
                }

                friend constexpr bool operator==(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires EqualityComparable<W>
                {
                    return lhs.m_Value == rhs.m_Value;
                }
                friend constexpr bool operator<(const Iterator& lhs,
                                                const Iterator& rhs)
                    requires TotallyOrdered<W>
                {
                    return lhs.m_Value < rhs.m_Value;
                }
                friend constexpr bool operator>(const Iterator& lhs,
                                                const Iterator& rhs)
                    requires TotallyOrdered<W>
                {
                    return rhs < lhs;
                }
                friend constexpr bool operator<=(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires TotallyOrdered<W>
                {
                    return !(rhs < lhs);
                }
                friend constexpr bool operator>=(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires TotallyOrdered<W>
                {
                    return !(lhs < rhs);
                }

                friend constexpr Iterator operator+(Iterator it, DifferenceType n)
                    requires Impl::Advanceable<W>
                {
                    it += n;
                    return it;
                }
                friend constexpr Iterator operator+(DifferenceType n, Iterator it)
                    requires Impl::Advanceable<W>
                {
                    it += n;
                    return it;
                }
                friend constexpr Iterator operator-(Iterator it, DifferenceType n)
                    requires Impl::Advanceable<W>
                {
                    it -= n;
                    return it;
                }
                friend constexpr DifferenceType operator-(const Iterator& lhs,
                                                          const Iterator& rhs)
                    requires Impl::Advanceable<W>
                {
                    if constexpr (Integral<W>)
                        return static_cast<DifferenceType>(
                            static_cast<DifferenceType>(lhs.m_Value)
                            - static_cast<DifferenceType>(rhs.m_Value));
                    else return lhs.m_Value - rhs.m_Value;
                }

              private:
                W m_Value = W();
            };

            class Sentinel
            {
              public:
                Sentinel() = default;
                constexpr explicit Sentinel(Bound bound)
                    : m_Bound(bound)
                {
                }

                friend constexpr bool operator==(const Iterator& it,
                                                 const Sentinel& sentinel)
                {
                    return it.Value() == sentinel.m_Bound;
                }
                friend constexpr typename Iterator::DifferenceType
                operator-(const Iterator& it, const Sentinel& sentinel)
                    requires SizedSentinelFor<Bound, W>
                {
                    return static_cast<typename Iterator::DifferenceType>(
                        it.Value() - sentinel.m_Bound);
                }
                friend constexpr typename Iterator::DifferenceType
                operator-(const Sentinel& sentinel, const Iterator& it)
                    requires SizedSentinelFor<Bound, W>
                {
                    return static_cast<typename Iterator::DifferenceType>(
                        sentinel.m_Bound - it.Value());
                }

              private:
                Bound m_Bound = Bound();
            };

            IotaView()
                requires DefaultInitializable<W>
            = default;
            constexpr explicit IotaView(W value)
                : m_Value(value)
            {
            }
            constexpr IotaView(TypeIdentityType<W>     value,
                               TypeIdentityType<Bound> bound)
                : m_Value(value)
                , m_Bound(bound)
            {
            }

            constexpr Iterator begin() const { return Iterator(m_Value); }
            constexpr auto     end() const
            {
                if constexpr (SameAs<W, Bound>) return Iterator(m_Bound);
                else if constexpr (SameAs<Bound, UnreachableSentinelType>)
                    return UnreachableSentinel;
                else return Sentinel(m_Bound);
            }

            constexpr auto Size() const
                requires(SameAs<W, Bound> && Impl::Advanceable<W>)
                     || (Integral<W> && Integral<Bound>)
                     || SizedSentinelFor<Bound, W>
            {
                if constexpr (Integral<W> && Integral<Bound>)
                    return static_cast<usize>(m_Bound)
                         - static_cast<usize>(m_Value);
                else return static_cast<usize>(m_Bound - m_Value);
            }

          private:
            W     m_Value = W();
            Bound m_Bound = Bound();
        };
        template <typename W, typename Bound>
        inline constexpr bool EnableBorrowedRange<IotaView<W, Bound>> = true;

        namespace Impl
        {
            struct IotaFn
            {
                template <typename W>
                    requires requires {
                        IotaView<DecayType<W>>(DeclVal<W>());
                    }
                constexpr auto operator()(W&& value) const
                {
                    return IotaView<DecayType<W>>(Forward<W>(value));
                }
                template <typename W, typename B>
                    requires requires {
                        IotaView<DecayType<W>, DecayType<B>>(DeclVal<W>(),
                                                             DeclVal<B>());
                    }
                constexpr auto operator()(W&& value, B&& bound) const
                {
                    return IotaView<DecayType<W>, DecayType<B>>(
                        Forward<W>(value), Forward<B>(bound));
                }
            };

            /// Views::Counted(it, n): the n elements starting at it.
            struct CountedFn
            {
                template <typename I>
                    requires RandomAccessIterator<DecayType<I>>
                constexpr auto
                operator()(I&& it, IteratorDifferenceType<DecayType<I>> n) const
                {
                    using It = DecayType<I>;
                    It first = Forward<I>(it);
                    It last  = first + n;
                    return Subrange<It>(Move(first), Move(last));
                }
            };
        } // namespace Impl

        // ======================================================================
        // FilterView

        template <InputRange V, typename P>
            requires View<V> && IsObjectV<P>
                  && IndirectUnaryPredicate<P, IteratorType<V>>
        class FilterView : public ViewInterface<FilterView<V, P>>
        {
          public:
            class Iterator
            {
              public:
                using IteratorConcept = ConditionalType<
                    BidirectionalRange<V>, BidirectionalIteratorTag,
                    ConditionalType<ForwardRange<V>, ForwardIteratorTag,
                                    InputIteratorTag>>;
                using ValueType      = RangeValueType<V>;
                using DifferenceType = RangeDifferenceType<V>;

                Iterator()
                    requires DefaultInitializable<IteratorType<V>>
                = default;
                constexpr Iterator(FilterView& parent, IteratorType<V> current)
                    : m_Parent(AddressOf(parent))
                    , m_Current(Move(current))
                {
                }

                constexpr const IteratorType<V>& Base() const&
                {
                    return m_Current;
                }
                constexpr IteratorType<V> Base() && { return Move(m_Current); }

                constexpr RangeReferenceType<V> operator*() const
                {
                    return *m_Current;
                }

                constexpr Iterator& operator++()
                {
                    ++m_Current;
                    m_Current = m_Parent->FindNext(Move(m_Current));
                    return *this;
                }
                constexpr void operator++(int)
                    requires(!ForwardRange<V>)
                {
                    ++*this;
                }
                constexpr Iterator operator++(int)
                    requires ForwardRange<V>
                {
                    Iterator copy = *this;
                    ++*this;
                    return copy;
                }
                constexpr Iterator& operator--()
                    requires BidirectionalRange<V>
                {
                    do --m_Current;
                    while (!Invoke(*m_Parent->m_Predicate, *m_Current));
                    return *this;
                }
                constexpr Iterator operator--(int)
                    requires BidirectionalRange<V>
                {
                    Iterator copy = *this;
                    --*this;
                    return copy;
                }

                friend constexpr bool operator==(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires EqualityComparable<IteratorType<V>>
                {
                    return lhs.m_Current == rhs.m_Current;
                }

              private:
                FilterView*     m_Parent  = nullptr;
                IteratorType<V> m_Current = IteratorType<V>();
            };

            class Sentinel
            {
              public:
                Sentinel() = default;
                constexpr explicit Sentinel(FilterView& parent)
                    : m_End(Ranges::End(parent.m_Base))
                {
                }

                constexpr SentinelType<V> Base() const { return m_End; }

                friend constexpr bool     operator==(const Iterator& it,
                                                 const Sentinel& sentinel)
                {
                    return it.Base() == sentinel.m_End;
                }

              private:
                SentinelType<V> m_End = SentinelType<V>();
            };

            FilterView()
                requires DefaultInitializable<V> && DefaultInitializable<P>
            = default;
            constexpr FilterView(V base, P predicate)
                : m_Base(Move(base))
                , m_Predicate(InPlace, Move(predicate))
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V        Base() && { return Move(m_Base); }
            constexpr const P& Pred() const { return *m_Predicate; }

            constexpr Iterator begin()
            {
                return Iterator(*this, FindNext(Ranges::Begin(m_Base)));
            }
            constexpr auto end()
            {
                if constexpr (CommonRange<V>)
                    return Iterator(*this, Ranges::End(m_Base));
                else return Sentinel(*this);
            }

          private:
            constexpr IteratorType<V> FindNext(IteratorType<V> it)
            {
                auto last = Ranges::End(m_Base);
                while (it != last && !Invoke(*m_Predicate, *it)) ++it;
                return it;
            }

            V                   m_Base = V();
            Impl::MovableBox<P> m_Predicate;
        };
        template <typename R, typename P>
        FilterView(R&&, P) -> FilterView<AllType<R>, P>;

        namespace Impl
        {
            struct FilterFn
            {
                template <ViewableRange R, typename P>
                    requires requires {
                        typename FilterView<AllType<R>, DecayType<P>>;
                    }
                constexpr auto operator()(R&& range, P&& predicate) const
                {
                    return FilterView<AllType<R>, DecayType<P>>(
                        Views::All(Forward<R>(range)), Forward<P>(predicate));
                }
                template <typename P>
                    requires(!Range<P>)
                constexpr auto operator()(P&& predicate) const
                {
                    return Partial<FilterFn, DecayType<P>>(
                        Forward<P>(predicate));
                }
            };
        } // namespace Impl

        // ======================================================================
        // TransformView

        template <InputRange V, typename F>
            requires View<V> && IsObjectV<F> && MoveConstructible<F>
                  && RegularInvocable<F&, RangeReferenceType<V>>
                  && ::Prism::Details::CanReference<
                      InvokeResultType<F&, RangeReferenceType<V>>>
        class TransformView : public ViewInterface<TransformView<V, F>>
        {
          public:
            template <bool Const>
            class Iterator
            {
                using Parent    = Impl::MaybeConst<Const, TransformView>;
                using BaseRange = Impl::MaybeConst<Const, V>;
                using BaseIter  = IteratorType<BaseRange>;
                using Function  = ConditionalType<Const, const F&, F&>;

              public:
                using IteratorConcept = Impl::IteratorConceptOf<BaseRange>;
                using ValueType       = RemoveCvRefType<
                          InvokeResultType<Function, RangeReferenceType<BaseRange>>>;
                using DifferenceType = RangeDifferenceType<BaseRange>;

                Iterator()
                    requires DefaultInitializable<BaseIter>
                = default;
                constexpr Iterator(Parent& parent, BaseIter current)
                    : m_Parent(AddressOf(parent))
                    , m_Current(Move(current))
                {
                }
                constexpr Iterator(Iterator<!Const> other)
                    requires Const
                          && ConvertibleTo<IteratorType<V>, BaseIter>
                    : m_Parent(other.m_Parent)
                    , m_Current(Move(other.m_Current))
                {
                }

                constexpr const BaseIter& Base() const& { return m_Current; }
                constexpr BaseIter        Base() && { return Move(m_Current); }

                constexpr decltype(auto)  operator*() const
                {
                    return Invoke(*m_Parent->m_Function, *m_Current);
                }

                constexpr Iterator& operator++()
                {
                    ++m_Current;
                    return *this;
                }
                constexpr void operator++(int)
                    requires(!ForwardRange<BaseRange>)
                {
                    ++m_Current;
                }
                constexpr Iterator operator++(int)
                    requires ForwardRange<BaseRange>
                {
                    Iterator copy = *this;
                    ++*this;
                    return copy;
                }
                constexpr Iterator& operator--()
                    requires BidirectionalRange<BaseRange>
                {
                    --m_Current;
                    return *this;
                }
                constexpr Iterator operator--(int)
                    requires BidirectionalRange<BaseRange>
                {
                    Iterator copy = *this;
                    --*this;
                    return copy;
                }
                constexpr Iterator& operator+=(DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    m_Current += n;
                    return *this;
                }
                constexpr Iterator& operator-=(DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    m_Current -= n;
                    return *this;
                }
                constexpr decltype(auto) operator[](DifferenceType n) const
                    requires RandomAccessRange<BaseRange>
                {
                    return Invoke(*m_Parent->m_Function, m_Current[n]);
                }

                friend constexpr bool operator==(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires EqualityComparable<BaseIter>
                {
                    return lhs.m_Current == rhs.m_Current;
                }
                friend constexpr bool operator<(const Iterator& lhs,
                                                const Iterator& rhs)
                    requires RandomAccessRange<BaseRange>
                {
                    return lhs.m_Current < rhs.m_Current;
                }
                friend constexpr bool operator>(const Iterator& lhs,
                                                const Iterator& rhs)
                    requires RandomAccessRange<BaseRange>
                {
                    return rhs < lhs;
                }
                friend constexpr bool operator<=(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires RandomAccessRange<BaseRange>
                {
                    return !(rhs < lhs);
                }
                friend constexpr bool operator>=(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires RandomAccessRange<BaseRange>
                {
                    return !(lhs < rhs);
                }
                friend constexpr Iterator operator+(Iterator it, DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    it += n;
                    return it;
                }
                friend constexpr Iterator operator+(DifferenceType n, Iterator it)
                    requires RandomAccessRange<BaseRange>
                {
                    it += n;
                    return it;
                }
                friend constexpr Iterator operator-(Iterator it, DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    it -= n;
                    return it;
                }
                friend constexpr DifferenceType operator-(const Iterator& lhs,
                                                          const Iterator& rhs)
                    requires SizedSentinelFor<BaseIter, BaseIter>
                {
                    return lhs.m_Current - rhs.m_Current;
                }

              private:
                template <bool>
                friend class Iterator;

                Parent*  m_Parent  = nullptr;
                BaseIter m_Current = BaseIter();
            };

            template <bool Const>
            class Sentinel
            {
                using BaseRange = Impl::MaybeConst<Const, V>;
                using BaseSent  = SentinelType<BaseRange>;

              public:
                Sentinel() = default;
                constexpr explicit Sentinel(BaseSent end)
                    : m_End(Move(end))
                {
                }
                constexpr Sentinel(Sentinel<!Const> other)
                    requires Const
                          && ConvertibleTo<SentinelType<V>, BaseSent>
                    : m_End(Move(other.m_End))
                {
                }

                constexpr BaseSent Base() const { return m_End; }

                friend constexpr bool operator==(const Iterator<Const>& it,
                                                 const Sentinel& sentinel)
                {
                    return it.Base() == sentinel.m_End;
                }
                friend constexpr RangeDifferenceType<BaseRange>
                operator-(const Iterator<Const>& it, const Sentinel& sentinel)
                    requires SizedSentinelFor<BaseSent, IteratorType<BaseRange>>
                {
                    return it.Base() - sentinel.m_End;
                }
                friend constexpr RangeDifferenceType<BaseRange>
                operator-(const Sentinel& sentinel, const Iterator<Const>& it)
                    requires SizedSentinelFor<BaseSent, IteratorType<BaseRange>>
                {
                    return sentinel.m_End - it.Base();
                }

              private:
                template <bool>
                friend class Sentinel;

                BaseSent m_End = BaseSent();
            };

            TransformView()
                requires DefaultInitializable<V> && DefaultInitializable<F>
            = default;
            constexpr TransformView(V base, F function)
                : m_Base(Move(base))
                , m_Function(InPlace, Move(function))
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V               Base() && { return Move(m_Base); }

            constexpr Iterator<false> begin()
            {
                return Iterator<false>(*this, Ranges::Begin(m_Base));
            }
            constexpr Iterator<true> begin() const
                requires Range<const V>
                      && RegularInvocable<const F&, RangeReferenceType<const V>>
            {
                return Iterator<true>(*this, Ranges::Begin(m_Base));
            }
            constexpr auto end()
            {
                if constexpr (CommonRange<V>)
                    return Iterator<false>(*this, Ranges::End(m_Base));
                else return Sentinel<false>(Ranges::End(m_Base));
            }
            constexpr auto end() const
                requires Range<const V>
                      && RegularInvocable<const F&, RangeReferenceType<const V>>
            {
                if constexpr (CommonRange<const V>)
                    return Iterator<true>(*this, Ranges::End(m_Base));
                else return Sentinel<true>(Ranges::End(m_Base));
            }

            constexpr auto Size()
                requires SizedRange<V>
            {
                return Ranges::Size(m_Base);
            }
            constexpr auto Size() const
                requires SizedRange<const V>
            {
                return Ranges::Size(m_Base);
            }

          private:
            V                   m_Base = V();
            Impl::MovableBox<F> m_Function;
        };
        template <typename R, typename F>
        TransformView(R&&, F) -> TransformView<AllType<R>, F>;

        namespace Impl
        {
            struct TransformFn
            {
                template <ViewableRange R, typename F>
                    requires requires {
                        typename TransformView<AllType<R>, DecayType<F>>;
                    }
                constexpr auto operator()(R&& range, F&& function) const
                {
                    return TransformView<AllType<R>, DecayType<F>>(
                        Views::All(Forward<R>(range)), Forward<F>(function));
                }
                template <typename F>
                    requires(!Range<F>)
                constexpr auto operator()(F&& function) const
                {
                    return Partial<TransformFn, DecayType<F>>(
                        Forward<F>(function));
                }
            };
        } // namespace Impl

        // ======================================================================
        // TakeView

        template <View V>
        class TakeView : public ViewInterface<TakeView<V>>
        {
            template <bool Const>
            static constexpr bool UsesBaseIterators
                = SizedRange<Impl::MaybeConst<Const, V>>
               && RandomAccessRange<Impl::MaybeConst<Const, V>>;

          public:
            /// Iterator for ranges that can't just be cut at begin() + count.
            template <bool Const>
            class Iterator
            {
                using BaseRange = Impl::MaybeConst<Const, V>;
                using BaseIter  = IteratorType<BaseRange>;

              public:
                using IteratorConcept
                    = ConditionalType<ForwardRange<BaseRange>,
                                      ForwardIteratorTag, InputIteratorTag>;
                using ValueType      = RangeValueType<BaseRange>;
                using DifferenceType = RangeDifferenceType<BaseRange>;

                Iterator()
                    requires DefaultInitializable<BaseIter>
                = default;
                constexpr Iterator(BaseIter current, DifferenceType count)
                    : m_Current(Move(current))
                    , m_Count(count)
                {
                }
                constexpr Iterator(Iterator<!Const> other)
                    requires Const
                          && ConvertibleTo<IteratorType<V>, BaseIter>
                    : m_Current(Move(other.m_Current))
                    , m_Count(other.m_Count)
                {
                }

                constexpr const BaseIter& Base() const& { return m_Current; }
                constexpr DifferenceType  Count() const { return m_Count; }

                constexpr decltype(auto)  operator*() const
                {
                    return *m_Current;
                }

                constexpr Iterator& operator++()
                {
                    ++m_Current;
                    --m_Count;
                    return *this;
                }
                constexpr void operator++(int)
                    requires(!ForwardRange<BaseRange>)
                {
                    ++*this;
                }
                constexpr Iterator operator++(int)
                    requires ForwardRange<BaseRange>
                {
                    Iterator copy = *this;
                    ++*this;
                    return copy;
                }

                friend constexpr bool operator==(const Iterator& lhs,
                                                 const Iterator& rhs)
                {
                    return lhs.m_Count == rhs.m_Count;
                }

              private:
                template <bool>
                friend class Iterator;

                BaseIter       m_Current = BaseIter();
                DifferenceType m_Count   = 0;
            };

            template <bool Const>
            class Sentinel
            {
                using BaseRange = Impl::MaybeConst<Const, V>;
                using BaseSent  = SentinelType<BaseRange>;

              public:
                Sentinel() = default;
                constexpr explicit Sentinel(BaseSent end)
                    : m_End(Move(end))
                {
                }

                constexpr BaseSent Base() const { return m_End; }

                friend constexpr bool operator==(const Iterator<Const>& it,
                                                 const Sentinel& sentinel)
                {
                    return it.Count() == 0 || it.Base() == sentinel.m_End;
                }

              private:
                BaseSent m_End = BaseSent();
            };

            TakeView()
                requires DefaultInitializable<V>
            = default;
            constexpr TakeView(V base, RangeDifferenceType<V> count)
                : m_Base(Move(base))
                , m_Count(count)
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V    Base() && { return Move(m_Base); }

            constexpr auto begin() { return BeginImpl<false>(*this); }
            constexpr auto begin() const
                requires Range<const V>
            {
                return BeginImpl<true>(*this);
            }
            constexpr auto end() { return EndImpl<false>(*this); }
            constexpr auto end() const
                requires Range<const V>
            {
                return EndImpl<true>(*this);
            }

            constexpr auto Size()
                requires SizedRange<V>
            {
                auto size = Ranges::Size(m_Base);
                auto n    = static_cast<decltype(size)>(m_Count);
                return n < size ? n : size;
            }
            constexpr auto Size() const
                requires SizedRange<const V>
            {
                auto size = Ranges::Size(m_Base);
                auto n    = static_cast<decltype(size)>(m_Count);
                return n < size ? n : size;
            }

          private:
            template <bool Const, typename Self>
            static constexpr auto BeginImpl(Self& self)
            {
                if constexpr (UsesBaseIterators<Const>)
                    return Ranges::Begin(self.m_Base);
                else
                    return Iterator<Const>(Ranges::Begin(self.m_Base),
                                           self.m_Count);
            }
            template <bool Const, typename Self>
            static constexpr auto EndImpl(Self& self)
            {
                if constexpr (UsesBaseIterators<Const>)
                {
                    using Difference
                        = RangeDifferenceType<Impl::MaybeConst<Const, V>>;
                    auto size
                        = static_cast<Difference>(Ranges::Size(self.m_Base));
                    auto count = static_cast<Difference>(self.m_Count);
                    return Ranges::Begin(self.m_Base)
                         + (count < size ? count : size);
                }
                else return Sentinel<Const>(Ranges::End(self.m_Base));
            }

            V                      m_Base  = V();
            RangeDifferenceType<V> m_Count = 0;
        };
        template <typename R>
        TakeView(R&&, RangeDifferenceType<R>) -> TakeView<AllType<R>>;
        template <typename T>
        inline constexpr bool EnableBorrowedRange<TakeView<T>>
            = EnableBorrowedRange<T>;

        namespace Impl
        {
            struct TakeFn
            {
                template <ViewableRange R>
                constexpr auto operator()(R&& range,
                                          RangeDifferenceType<R> count) const
                {
                    return TakeView<AllType<R>>(Views::All(Forward<R>(range)),
                                                count);
                }
                template <typename N>
                    requires Integral<N>
                constexpr auto operator()(N count) const
                {
                    return Partial<TakeFn, N>(count);
                }
            };
        } // namespace Impl

        // ======================================================================
        // TakeWhileView

        template <View V, typename P>
            requires InputRange<V> && IsObjectV<P>
                  && IndirectUnaryPredicate<const P, IteratorType<V>>
        class TakeWhileView : public ViewInterface<TakeWhileView<V, P>>
        {
          public:
            template <bool Const>
            class Sentinel
            {
                using BaseRange = Impl::MaybeConst<Const, V>;
                using BaseSent  = SentinelType<BaseRange>;

              public:
                Sentinel() = default;
                constexpr Sentinel(BaseSent end, const P* predicate)
                    : m_End(Move(end))
                    , m_Predicate(predicate)
                {
                }

                constexpr BaseSent Base() const { return m_End; }

                friend constexpr bool
                operator==(const IteratorType<BaseRange>& it,
                           const Sentinel&                sentinel)
                {
                    return it == sentinel.m_End
                        || !Invoke(*sentinel.m_Predicate, *it);
                }

              private:
                BaseSent m_End       = BaseSent();
                const P* m_Predicate = nullptr;
            };

            TakeWhileView()
                requires DefaultInitializable<V> && DefaultInitializable<P>
            = default;
            constexpr TakeWhileView(V base, P predicate)
                : m_Base(Move(base))
                , m_Predicate(InPlace, Move(predicate))
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V        Base() && { return Move(m_Base); }
            constexpr const P& Pred() const { return *m_Predicate; }

            constexpr auto     begin() { return Ranges::Begin(m_Base); }
            constexpr auto     begin() const
                requires Range<const V>
                      && IndirectUnaryPredicate<const P, IteratorType<const V>>
            {
                return Ranges::Begin(m_Base);
            }
            constexpr auto end()
            {
                return Sentinel<false>(Ranges::End(m_Base),
                                       AddressOf(*m_Predicate));
            }
            constexpr auto end() const
                requires Range<const V>
                      && IndirectUnaryPredicate<const P, IteratorType<const V>>
            {
                return Sentinel<true>(Ranges::End(m_Base),
                                      AddressOf(*m_Predicate));
            }

          private:
            V                   m_Base = V();
            Impl::MovableBox<P> m_Predicate;
        };
        template <typename R, typename P>
        TakeWhileView(R&&, P) -> TakeWhileView<AllType<R>, P>;

        namespace Impl
        {
            struct TakeWhileFn
            {
                template <ViewableRange R, typename P>
                    requires requires {
                        typename TakeWhileView<AllType<R>, DecayType<P>>;
                    }
                constexpr auto operator()(R&& range, P&& predicate) const
                {
                    return TakeWhileView<AllType<R>, DecayType<P>>(
                        Views::All(Forward<R>(range)), Forward<P>(predicate));
                }
                template <typename P>
                    requires(!Range<P>)
                constexpr auto operator()(P&& predicate) const
                {
                    return Partial<TakeWhileFn, DecayType<P>>(
                        Forward<P>(predicate));
                }
            };
        } // namespace Impl

        // ======================================================================
        // DropView, DropWhileView

        template <View V>
        class DropView : public ViewInterface<DropView<V>>
        {
          public:
            DropView()
                requires DefaultInitializable<V>
            = default;
            constexpr DropView(V base, RangeDifferenceType<V> count)
                : m_Base(Move(base))
                , m_Count(count)
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V    Base() && { return Move(m_Base); }

            constexpr auto begin()
            {
                return Ranges::Next(Ranges::Begin(m_Base), m_Count,
                                    Ranges::End(m_Base));
            }
            constexpr auto begin() const
                requires Range<const V>
            {
                return Ranges::Next(Ranges::Begin(m_Base),
                                    static_cast<RangeDifferenceType<const V>>(
                                        m_Count),
                                    Ranges::End(m_Base));
            }
            constexpr auto end() { return Ranges::End(m_Base); }
            constexpr auto end() const
                requires Range<const V>
            {
                return Ranges::End(m_Base);
            }

            constexpr auto Size()
                requires SizedRange<V>
            {
                auto size = Ranges::Size(m_Base);
                auto n    = static_cast<decltype(size)>(m_Count);
                return size < n ? decltype(size)(0) : size - n;
            }
            constexpr auto Size() const
                requires SizedRange<const V>
            {
                auto size = Ranges::Size(m_Base);
                auto n    = static_cast<decltype(size)>(m_Count);
                return size < n ? decltype(size)(0) : size - n;
            }

          private:
            V                      m_Base  = V();
            RangeDifferenceType<V> m_Count = 0;
        };
        template <typename R>
        DropView(R&&, RangeDifferenceType<R>) -> DropView<AllType<R>>;
        template <typename T>
        inline constexpr bool EnableBorrowedRange<DropView<T>>
            = EnableBorrowedRange<T>;

        template <View V, typename P>
            requires InputRange<V> && IsObjectV<P>
                  && IndirectUnaryPredicate<const P, IteratorType<V>>
        class DropWhileView : public ViewInterface<DropWhileView<V, P>>
        {
          public:
            DropWhileView()
                requires DefaultInitializable<V> && DefaultInitializable<P>
            = default;
            constexpr DropWhileView(V base, P predicate)
                : m_Base(Move(base))
                , m_Predicate(InPlace, Move(predicate))
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V        Base() && { return Move(m_Base); }
            constexpr const P& Pred() const { return *m_Predicate; }

            constexpr auto     begin()
            {
                auto it   = Ranges::Begin(m_Base);
                auto last = Ranges::End(m_Base);
                while (it != last && Invoke(*m_Predicate, *it)) ++it;
                return it;
            }
            constexpr auto end() { return Ranges::End(m_Base); }

          private:
            V                   m_Base = V();
            Impl::MovableBox<P> m_Predicate;
        };
        template <typename R, typename P>
        DropWhileView(R&&, P) -> DropWhileView<AllType<R>, P>;
        template <typename T, typename P>
        inline constexpr bool EnableBorrowedRange<DropWhileView<T, P>>
            = EnableBorrowedRange<T>;

        namespace Impl
        {
            struct DropFn
            {
                template <ViewableRange R>
                constexpr auto operator()(R&& range,
                                          RangeDifferenceType<R> count) const
                {
                    return DropView<AllType<R>>(Views::All(Forward<R>(range)),
                                                count);
                }
                template <typename N>
                    requires Integral<N>
                constexpr auto operator()(N count) const
                {
                    return Partial<DropFn, N>(count);
                }
            };

            struct DropWhileFn
            {
                template <ViewableRange R, typename P>
                    requires requires {
                        typename DropWhileView<AllType<R>, DecayType<P>>;
                    }
                constexpr auto operator()(R&& range, P&& predicate) const
                {
                    return DropWhileView<AllType<R>, DecayType<P>>(
                        Views::All(Forward<R>(range)), Forward<P>(predicate));
                }
                template <typename P>
                    requires(!Range<P>)
                constexpr auto operator()(P&& predicate) const
                {
                    return Partial<DropWhileFn, DecayType<P>>(
                        Forward<P>(predicate));
                }
            };
        } // namespace Impl

        // ======================================================================
        // ReverseView

        template <View V>
            requires BidirectionalRange<V>
        class ReverseView : public ViewInterface<ReverseView<V>>
        {
          public:
            ReverseView()
                requires DefaultInitializable<V>
            = default;
            constexpr explicit ReverseView(V base)
                : m_Base(Move(base))
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V                                  Base() && { return Move(m_Base); }

            constexpr ReverseIterator<IteratorType<V>> begin()
            {
                if constexpr (CommonRange<V>)
                    return ReverseIterator<IteratorType<V>>(
                        Ranges::End(m_Base));
                else
                    return ReverseIterator<IteratorType<V>>(Ranges::Next(
                        Ranges::Begin(m_Base), Ranges::End(m_Base)));
            }
            constexpr auto begin() const
                requires CommonRange<const V>
            {
                return ReverseIterator<IteratorType<const V>>(
                    Ranges::End(m_Base));
            }
            constexpr ReverseIterator<IteratorType<V>> end()
            {
                return ReverseIterator<IteratorType<V>>(Ranges::Begin(m_Base));
            }
            constexpr auto end() const
                requires CommonRange<const V>
            {
                return ReverseIterator<IteratorType<const V>>(
                    Ranges::Begin(m_Base));
            }

            constexpr auto Size()
                requires SizedRange<V>
            {
                return Ranges::Size(m_Base);
            }
            constexpr auto Size() const
                requires SizedRange<const V>
            {
                return Ranges::Size(m_Base);
            }

          private:
            V m_Base = V();
        };
        template <typename R>
        ReverseView(R&&) -> ReverseView<AllType<R>>;
        template <typename T>
        inline constexpr bool EnableBorrowedRange<ReverseView<T>>
            = EnableBorrowedRange<T>;

        namespace Impl
        {
            template <typename T>
            inline constexpr bool IsReverseView = false;
            template <typename V>
            inline constexpr bool IsReverseView<ReverseView<V>> = true;

            struct ReverseFn : RangeAdaptorClosure<ReverseFn>
            {
                template <ViewableRange R>
                    requires IsReverseView<RemoveCvRefType<R>>
                          || requires { typename ReverseView<AllType<R>>; }
                constexpr auto operator()(R&& range) const
                {
                    if constexpr (IsReverseView<RemoveCvRefType<R>>)
                        return Forward<R>(range).Base();
                    else
                        return ReverseView<AllType<R>>(
                            Views::All(Forward<R>(range)));
                }
            };
        } // namespace Impl

        // ======================================================================
        // JoinView

        /// Flattens a range of ranges (std::ranges::join_view). The inner
        /// ranges must be references with default-constructible iterators.
        template <InputRange V>
            requires View<V> && InputRange<RangeReferenceType<V>>
                  && IsReferenceV<RangeReferenceType<V>>
                  && DefaultInitializable<IteratorType<RangeReferenceType<V>>>
        class JoinView : public ViewInterface<JoinView<V>>
        {
            template <bool Const>
            using InnerRange
                = RangeReferenceType<Impl::MaybeConst<Const, V>>;

          public:
            template <bool Const>
            class Iterator
            {
                using Parent    = Impl::MaybeConst<Const, JoinView>;
                using Outer     = Impl::MaybeConst<Const, V>;
                using OuterIter = IteratorType<Outer>;
                using Inner     = InnerRange<Const>;
                using InnerIter = IteratorType<Inner>;

              public:
                using IteratorConcept = ConditionalType<
                    ForwardRange<Outer> && ForwardRange<Inner>,
                    ForwardIteratorTag, InputIteratorTag>;
                using ValueType      = RangeValueType<Inner>;
                using DifferenceType = CommonTypeType<RangeDifferenceType<Outer>,
                                                      RangeDifferenceType<Inner>>;

                Iterator()
                    requires DefaultInitializable<OuterIter>
                = default;
                constexpr Iterator(Parent& parent, OuterIter outer)
                    : m_Parent(AddressOf(parent))
                    , m_Outer(Move(outer))
                {
                    Satisfy();
                }
                constexpr Iterator(Iterator<!Const> other)
                    requires Const
                          && ConvertibleTo<IteratorType<V>, OuterIter>
                          && ConvertibleTo<
                                 IteratorType<InnerRange<false>>, InnerIter>
                    : m_Parent(other.m_Parent)
                    , m_Outer(Move(other.m_Outer))
                    , m_Inner(Move(other.m_Inner))
                {
                }

                constexpr const OuterIter& OuterBase() const { return m_Outer; }

                constexpr decltype(auto)   operator*() const { return *m_Inner; }

                constexpr Iterator&        operator++()
                {
                    if (++m_Inner == Ranges::End(*m_Outer))
                    {
                        ++m_Outer;
                        Satisfy();
                    }
                    return *this;
                }
                constexpr void operator++(int)
                    requires(!ForwardRange<Outer> || !ForwardRange<Inner>)
                {
                    ++*this;
                }
                constexpr Iterator operator++(int)
                    requires ForwardRange<Outer> && ForwardRange<Inner>
                {
                    Iterator copy = *this;
                    ++*this;
                    return copy;
                }

                friend constexpr bool operator==(const Iterator& lhs,
                                                 const Iterator& rhs)
                    requires ForwardRange<Outer>
                          && EqualityComparable<InnerIter>
                {
                    return lhs.m_Outer == rhs.m_Outer
                        && lhs.m_Inner == rhs.m_Inner;
                }

              private:
                template <bool>
                friend class Iterator;

                // Moves to the first element of the next non-empty inner range.
                constexpr void Satisfy()
                {
                    auto last = Ranges::End(m_Parent->m_Base);
                    for (; m_Outer != last; ++m_Outer)
                    {
                        auto&& inner = *m_Outer;
                        m_Inner      = Ranges::Begin(inner);
                        if (m_Inner != Ranges::End(inner)) return;
                    }
                    m_Inner = InnerIter();
                }

                Parent*   m_Parent = nullptr;
                OuterIter m_Outer  = OuterIter();
                InnerIter m_Inner  = InnerIter();
            };

            template <bool Const>
            class Sentinel
            {
                using Outer    = Impl::MaybeConst<Const, V>;
                using BaseSent = SentinelType<Outer>;

              public:
                Sentinel() = default;
                constexpr explicit Sentinel(BaseSent end)
                    : m_End(Move(end))
                {
                }

                friend constexpr bool operator==(const Iterator<Const>& it,
                                                 const Sentinel& sentinel)
                {
                    return it.OuterBase() == sentinel.m_End;
                }

              private:
                BaseSent m_End = BaseSent();
            };

            JoinView()
                requires DefaultInitializable<V>
            = default;
            constexpr explicit JoinView(V base)
                : m_Base(Move(base))
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V               Base() && { return Move(m_Base); }

            constexpr Iterator<false> begin()
            {
                return Iterator<false>(*this, Ranges::Begin(m_Base));
            }
            constexpr Iterator<true> begin() const
                requires InputRange<const V>
                      && IsReferenceV<RangeReferenceType<const V>>
                      && InputRange<RangeReferenceType<const V>>
            {
                return Iterator<true>(*this, Ranges::Begin(m_Base));
            }
            constexpr auto end()
            {
                if constexpr (ForwardRange<V> && CommonRange<V>
                              && ForwardRange<InnerRange<false>>)
                    return Iterator<false>(*this, Ranges::End(m_Base));
                else return Sentinel<false>(Ranges::End(m_Base));
            }
            constexpr auto end() const
                requires InputRange<const V>
                      && IsReferenceV<RangeReferenceType<const V>>
                      && InputRange<RangeReferenceType<const V>>
            {
                if constexpr (ForwardRange<const V> && CommonRange<const V>
                              && ForwardRange<InnerRange<true>>)
                    return Iterator<true>(*this, Ranges::End(m_Base));
                else return Sentinel<true>(Ranges::End(m_Base));
            }

          private:
            V m_Base = V();
        };
        template <typename R>
        explicit JoinView(R&&) -> JoinView<AllType<R>>;

        namespace Impl
        {
            struct JoinFn : RangeAdaptorClosure<JoinFn>
            {
                template <ViewableRange R>
                    requires requires { typename JoinView<AllType<R>>; }
                constexpr auto operator()(R&& range) const
                {
                    return JoinView<AllType<R>>(Views::All(Forward<R>(range)));
                }
            };
        } // namespace Impl

        // ======================================================================
        // Elements / Keys / Values

        namespace Impl
        {
            /// Element N of a pair-like or tuple-like value: Key / Value
            /// members (KeyValuePair) for 0 / 1, otherwise Get<N>().
            template <typename T, usize N>
            concept HasElement
                = (N == 0 && requires(T& t) { t.Key; })
               || (N == 1 && requires(T& t) { t.Value; })
               || requires(T& t) { t.template Get<N>(); };

            template <usize N, typename T>
                requires HasElement<RemoveReferenceType<T>, N>
            constexpr decltype(auto) GetElement(T&& value)
            {
                if constexpr (N == 0 && requires { Forward<T>(value).Key; })
                    return (Forward<T>(value).Key);
                else if constexpr (N == 1
                                   && requires { Forward<T>(value).Value; })
                    return (Forward<T>(value).Value);
                else return Forward<T>(value).template Get<N>();
            }

            template <usize N>
            struct ElementFn
            {
                template <typename T>
                    requires HasElement<RemoveReferenceType<T>, N>
                constexpr decltype(auto) operator()(T&& value) const
                {
                    // Elements of a prvalue are returned by value, so they
                    // don't dangle.
                    if constexpr (IsLValueReferenceV<T>)
                        return Impl::GetElement<N>(value);
                    else
                    {
                        using Element = RemoveCvRefType<decltype(
                            Impl::GetElement<N>(Forward<T>(value)))>;
                        return static_cast<Element>(
                            Impl::GetElement<N>(Forward<T>(value)));
                    }
                }
            };

            template <usize N>
            struct ElementsFn : RangeAdaptorClosure<ElementsFn<N>>
            {
                template <ViewableRange R>
                    requires requires {
                        typename TransformView<AllType<R>, ElementFn<N>>;
                    }
                constexpr auto operator()(R&& range) const
                {
                    return TransformView<AllType<R>, ElementFn<N>>(
                        Views::All(Forward<R>(range)), ElementFn<N>{});
                }
            };
        } // namespace Impl

        // ======================================================================
        // EnumerateView

        /// What Views::Enumerate yields: `auto [index, value] = *it;`
        template <typename I, typename R>
        struct EnumerateElement
        {
            I Index;
            R Value;
        };

        template <View V>
            requires InputRange<V>
        class EnumerateView : public ViewInterface<EnumerateView<V>>
        {
          public:
            template <bool Const>
            class Iterator
            {
                using BaseRange = Impl::MaybeConst<Const, V>;
                using BaseIter  = IteratorType<BaseRange>;

              public:
                using IteratorConcept = Impl::IteratorConceptOf<BaseRange>;
                using DifferenceType  = RangeDifferenceType<BaseRange>;
                using ValueType
                    = EnumerateElement<DifferenceType, RangeValueType<BaseRange>>;
                using Reference = EnumerateElement<DifferenceType,
                                                   RangeReferenceType<BaseRange>>;

                Iterator()
                    requires DefaultInitializable<BaseIter>
                = default;
                constexpr Iterator(BaseIter current, DifferenceType index)
                    : m_Current(Move(current))
                    , m_Index(index)
                {
                }
                constexpr Iterator(Iterator<!Const> other)
                    requires Const
                          && ConvertibleTo<IteratorType<V>, BaseIter>
                    : m_Current(Move(other.m_Current))
                    , m_Index(other.m_Index)
                {
                }

                constexpr const BaseIter& Base() const& { return m_Current; }
                constexpr DifferenceType  Index() const { return m_Index; }

                constexpr Reference       operator*() const
                {
                    return Reference{m_Index, *m_Current};
                }

                constexpr Iterator& operator++()
                {
                    ++m_Current;
                    ++m_Index;
                    return *this;
                }
                constexpr void operator++(int)
                    requires(!ForwardRange<BaseRange>)
                {
                    ++*this;
                }
                constexpr Iterator operator++(int)
                    requires ForwardRange<BaseRange>
                {
                    Iterator copy = *this;
                    ++*this;
                    return copy;
                }
                constexpr Iterator& operator--()
                    requires BidirectionalRange<BaseRange>
                {
                    --m_Current;
                    --m_Index;
                    return *this;
                }
                constexpr Iterator operator--(int)
                    requires BidirectionalRange<BaseRange>
                {
                    Iterator copy = *this;
                    --*this;
                    return copy;
                }
                constexpr Iterator& operator+=(DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    m_Current += n;
                    m_Index += n;
                    return *this;
                }
                constexpr Iterator& operator-=(DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    m_Current -= n;
                    m_Index -= n;
                    return *this;
                }
                constexpr Reference operator[](DifferenceType n) const
                    requires RandomAccessRange<BaseRange>
                {
                    return Reference{m_Index + n, m_Current[n]};
                }

                friend constexpr bool operator==(const Iterator& lhs,
                                                 const Iterator& rhs)
                {
                    return lhs.m_Index == rhs.m_Index;
                }
                friend constexpr bool operator<(const Iterator& lhs,
                                                const Iterator& rhs)
                {
                    return lhs.m_Index < rhs.m_Index;
                }
                friend constexpr bool operator>(const Iterator& lhs,
                                                const Iterator& rhs)
                {
                    return rhs < lhs;
                }
                friend constexpr bool operator<=(const Iterator& lhs,
                                                 const Iterator& rhs)
                {
                    return !(rhs < lhs);
                }
                friend constexpr bool operator>=(const Iterator& lhs,
                                                 const Iterator& rhs)
                {
                    return !(lhs < rhs);
                }
                friend constexpr Iterator operator+(Iterator it, DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    it += n;
                    return it;
                }
                friend constexpr Iterator operator+(DifferenceType n, Iterator it)
                    requires RandomAccessRange<BaseRange>
                {
                    it += n;
                    return it;
                }
                friend constexpr Iterator operator-(Iterator it, DifferenceType n)
                    requires RandomAccessRange<BaseRange>
                {
                    it -= n;
                    return it;
                }
                friend constexpr DifferenceType operator-(const Iterator& lhs,
                                                          const Iterator& rhs)
                {
                    return lhs.m_Index - rhs.m_Index;
                }

              private:
                template <bool>
                friend class Iterator;

                BaseIter       m_Current = BaseIter();
                DifferenceType m_Index   = 0;
            };

            template <bool Const>
            class Sentinel
            {
                using BaseRange = Impl::MaybeConst<Const, V>;
                using BaseSent  = SentinelType<BaseRange>;

              public:
                Sentinel() = default;
                constexpr explicit Sentinel(BaseSent end)
                    : m_End(Move(end))
                {
                }

                friend constexpr bool operator==(const Iterator<Const>& it,
                                                 const Sentinel& sentinel)
                {
                    return it.Base() == sentinel.m_End;
                }

              private:
                BaseSent m_End = BaseSent();
            };

            EnumerateView()
                requires DefaultInitializable<V>
            = default;
            constexpr explicit EnumerateView(V base)
                : m_Base(Move(base))
            {
            }

            constexpr V Base() const&
                requires CopyConstructible<V>
            {
                return m_Base;
            }
            constexpr V    Base() && { return Move(m_Base); }

            constexpr auto begin()
            {
                return Iterator<false>(Ranges::Begin(m_Base), 0);
            }
            constexpr auto begin() const
                requires InputRange<const V>
            {
                return Iterator<true>(Ranges::Begin(m_Base), 0);
            }
            constexpr auto end()
            {
                if constexpr (CommonRange<V> && SizedRange<V>)
                    return Iterator<false>(
                        Ranges::End(m_Base),
                        static_cast<RangeDifferenceType<V>>(
                            Ranges::Size(m_Base)));
                else return Sentinel<false>(Ranges::End(m_Base));
            }
            constexpr auto end() const
                requires InputRange<const V>
            {
                if constexpr (CommonRange<const V> && SizedRange<const V>)
                    return Iterator<true>(
                        Ranges::End(m_Base),
                        static_cast<RangeDifferenceType<const V>>(
                            Ranges::Size(m_Base)));
                else return Sentinel<true>(Ranges::End(m_Base));
            }

            constexpr auto Size()
                requires SizedRange<V>
            {
                return Ranges::Size(m_Base);
            }
            constexpr auto Size() const
                requires SizedRange<const V>
            {
                return Ranges::Size(m_Base);
            }

          private:
            V m_Base = V();
        };
        template <typename R>
        EnumerateView(R&&) -> EnumerateView<AllType<R>>;
        template <typename T>
        inline constexpr bool EnableBorrowedRange<EnumerateView<T>>
            = EnableBorrowedRange<T>;

        namespace Impl
        {
            struct EnumerateFn : RangeAdaptorClosure<EnumerateFn>
            {
                template <ViewableRange R>
                    requires requires { typename EnumerateView<AllType<R>>; }
                constexpr auto operator()(R&& range) const
                {
                    return EnumerateView<AllType<R>>(
                        Views::All(Forward<R>(range)));
                }
            };
        } // namespace Impl

        // ======================================================================
        // Ranges::To

        /**
         * Copies a range into a new container (std::ranges::to), e.g.
         * `Ranges::To<Vector<int>>(view)` or `view | Ranges::To<Vector<int>>()`.
         * Elements are added with PushBack/push_back, or Insert/insert.
         */
        template <typename C, InputRange R>
            requires(!View<C>)
        constexpr C To(R&& range)
        {
            C result;
            if constexpr (SizedRange<R>
                          && requires(C& c, usize n) { c.Reserve(n); })
                result.Reserve(static_cast<usize>(Ranges::Size(range)));

            auto last = Ranges::End(range);
            for (auto it = Ranges::Begin(range); it != last; ++it)
            {
                if constexpr (requires { result.PushBack(*it); })
                    result.PushBack(*it);
                else if constexpr (requires { result.push_back(*it); })
                    result.push_back(*it);
                else if constexpr (requires { result.Insert(*it); })
                    result.Insert(*it);
                else result.insert(result.end(), *it);
            }
            return result;
        }

        namespace Impl
        {
            template <typename C>
            struct ToFn : RangeAdaptorClosure<ToFn<C>>
            {
                template <InputRange R>
                constexpr C operator()(R&& range) const
                {
                    return Ranges::To<C>(Forward<R>(range));
                }
            };
        } // namespace Impl

        template <typename C>
            requires(!View<C>)
        constexpr Impl::ToFn<C> To()
        {
            return {};
        }
    } // namespace Ranges

    namespace Views
    {
        template <typename T>
        inline constexpr Ranges::EmptyView<T>        Empty{};
        inline constexpr Ranges::Impl::SingleFn      Single{};
        inline constexpr Ranges::Impl::IotaFn        Iota{};
        inline constexpr Ranges::Impl::CountedFn     Counted{};
        inline constexpr Ranges::Impl::FilterFn      Filter{};
        inline constexpr Ranges::Impl::TransformFn   Transform{};
        inline constexpr Ranges::Impl::TakeFn        Take{};
        inline constexpr Ranges::Impl::TakeWhileFn   TakeWhile{};
        inline constexpr Ranges::Impl::DropFn        Drop{};
        inline constexpr Ranges::Impl::DropWhileFn   DropWhile{};
        inline constexpr Ranges::Impl::ReverseFn     Reverse{};
        inline constexpr Ranges::Impl::JoinFn        Join{};
        template <usize N>
        inline constexpr Ranges::Impl::ElementsFn<N> Elements{};
        inline constexpr Ranges::Impl::ElementsFn<0> Keys{};
        inline constexpr Ranges::Impl::ElementsFn<1> Values{};
        inline constexpr Ranges::Impl::EnumerateFn   Enumerate{};
    } // namespace Views
}; // namespace Prism
