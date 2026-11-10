/*
 * Created by v1tr10l7 on 09.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Containers/KeyValuePair.hpp>
#include <Prism/Containers/Span.hpp>
#include <Prism/Containers/Vector.hpp>
#include <Prism/Ranges/Ranges.hpp>
#include <Prism/String/StringView.hpp>
#include <Prism/Test/Test.hpp>

using namespace Prism;

// ----------------------------------------------------------------- concepts
static_assert(ContiguousIterator<int*>);
static_assert(ContiguousIterator<const int*>);
static_assert(RandomAccessIterator<ReverseIterator<int*>>);
static_assert(!ContiguousIterator<ReverseIterator<int*>>);
static_assert(SameAs<IteratorValueType<const int*>, int>);
static_assert(SameAs<IteratorDifferenceType<int*>, ptrdiff>);
static_assert(SentinelFor<UnreachableSentinelType, int*>);

static_assert(Ranges::ContiguousRange<Vector<int>>);
static_assert(Ranges::SizedRange<Vector<int>>);
static_assert(Ranges::ContiguousRange<int[4]>);
static_assert(Ranges::ContiguousRange<Span<int>>);
static_assert(Ranges::BorrowedRange<StringView>);
static_assert(Ranges::BorrowedRange<Vector<int>&>);
static_assert(!Ranges::BorrowedRange<Vector<int>>);
static_assert(Ranges::View<StringView>);
static_assert(Ranges::View<Span<int>>);
static_assert(!Ranges::View<Vector<int>>);
static_assert(Ranges::ViewableRange<Vector<int>&>);
static_assert(SameAs<Ranges::RangeValueType<Vector<int>>, int>);
static_assert(SameAs<Ranges::RangeReferenceType<const Vector<int>>, const int&>);

static_assert(ThreeWayComparable<int>);
static_assert(ThreeWayComparable<int, StrongOrdering>);
static_assert(SameAs<CommonComparisonCategoryType<StrongOrdering, WeakOrdering>,
                     WeakOrdering>);
static_assert(
    SameAs<CommonComparisonCategoryType<StrongOrdering, int>, void>);
static_assert(Predicate<bool (*)(int), int>);

template <typename R>
static Vector<int> Collect(R&& range)
{
    return Ranges::To<Vector<int>>(Forward<R>(range));
}

// ------------------------------------------------------------ range access
TEST_CASE(range_access)
{
    int arr[3] = {1, 2, 3};
    EXPECT_EQ(Ranges::Size(arr), usize(3));
    EXPECT(Ranges::Data(arr) == arr);
    EXPECT(Ranges::Begin(arr) == arr);
    EXPECT(Ranges::End(arr) == arr + 3);
    EXPECT(!Ranges::Empty(arr));

    Vector<int> v{1, 2, 3, 4};
    EXPECT_EQ(Ranges::Size(v), usize(4));
    EXPECT_EQ(Ranges::SSize(v), ptrdiff(4));
    EXPECT(Ranges::Data(v) == v.Raw());
    EXPECT_EQ(*Ranges::RBegin(v), 4);
    EXPECT_EQ(Ranges::Distance(v), ptrdiff(4));

    auto it = Ranges::Begin(v);
    Ranges::Advance(it, 2);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(*Ranges::Next(Ranges::Begin(v)), 2);
    EXPECT_EQ(*Ranges::Prev(Ranges::End(v)), 4);
    // Bounded advance stops at the end and reports the missing steps.
    auto bounded = Ranges::Begin(v);
    EXPECT_EQ(Ranges::Advance(bounded, 10, Ranges::End(v)), ptrdiff(6));
    EXPECT(bounded == Ranges::End(v));
}

TEST_CASE(span_from_range)
{
    Vector<int> v{1, 2, 3};
    Span<int>   span(v);
    EXPECT_EQ(span.Size(), usize(3));
    EXPECT_EQ(span[2], 3);
}

TEST_CASE(subrange)
{
    Vector<int>     v{1, 2, 3, 4, 5, 6};
    Ranges::Subrange sub(v.begin() + 1, v.end() - 1);
    EXPECT_EQ(sub.Size(), usize(4));
    EXPECT_EQ(sub.Front(), 2);
    EXPECT_EQ(sub.Back(), 5);
    EXPECT_EQ(sub[1], 3);
    EXPECT(!sub.Empty());
    EXPECT(static_cast<bool>(sub));
    EXPECT_EQ(sub.Next(2).Front(), 4);
}

// ------------------------------------------------------------------- views
TEST_CASE(filter_transform_pipe)
{
    Vector<int> v{1, 2, 3, 4, 5, 6};
    auto        out = v | Views::Filter([](int x) { return x % 2 == 0; })
             | Views::Transform([](int x) { return x * x; })
             | Ranges::To<Vector<int>>();
    EXPECT_EQ(out.Size(), usize(3));
    EXPECT_EQ(out[0], 4);
    EXPECT_EQ(out[1], 16);
    EXPECT_EQ(out[2], 36);

    // A composed adaptor can be stored and reused.
    auto evenSquares = Views::Filter([](int x) { return x % 2 == 0; })
                     | Views::Transform([](int x) { return x * x; });
    EXPECT_EQ(Collect(v | evenSquares).Size(), usize(3));
}

TEST_CASE(transform_random_access)
{
    Vector<int> v{1, 2, 3};
    auto        doubled = Views::Transform(v, [](int x) { return x * 2; });
    static_assert(Ranges::RandomAccessRange<decltype(doubled)>);
    EXPECT_EQ(doubled.Size(), usize(3));
    EXPECT_EQ(doubled[2], 6);
    EXPECT_EQ(*(doubled.begin() + 1), 4);
}

TEST_CASE(iota_take_drop)
{
    auto first = Collect(Views::Iota(0) | Views::Take(5));
    EXPECT_EQ(first.Size(), usize(5));
    EXPECT_EQ(first[0], 0);
    EXPECT_EQ(first[4], 4);

    auto bounded = Views::Iota(0, 10);
    EXPECT_EQ(bounded.Size(), usize(10));

    auto tail = Collect(bounded | Views::Drop(7));
    EXPECT_EQ(tail.Size(), usize(3));
    EXPECT_EQ(tail[0], 7);
    EXPECT_EQ(tail[2], 9);

    // Taking more than there is takes everything.
    EXPECT_EQ(Collect(Views::Iota(0, 3) | Views::Take(10)).Size(), usize(3));
    EXPECT_EQ(Collect(Views::Iota(0, 3) | Views::Drop(10)).Size(), usize(0));
}

TEST_CASE(take_while_drop_while)
{
    Vector<int> v{1, 2, 3, 10, 4, 5};
    auto        small = [](int x) { return x < 5; };

    auto        head  = Collect(v | Views::TakeWhile(small));
    EXPECT_EQ(head.Size(), usize(3));
    EXPECT_EQ(head[2], 3);

    auto rest = Collect(v | Views::DropWhile(small));
    EXPECT_EQ(rest.Size(), usize(3));
    EXPECT_EQ(rest[0], 10);
}

TEST_CASE(reverse)
{
    Vector<int> v{1, 2, 3, 4};
    auto        reversed = Collect(v | Views::Reverse);
    EXPECT_EQ(reversed.Size(), usize(4));
    EXPECT_EQ(reversed[0], 4);
    EXPECT_EQ(reversed[3], 1);

    // Reversing twice gives the original order back.
    auto twice = Collect(v | Views::Reverse | Views::Reverse);
    EXPECT_EQ(twice[0], 1);
    EXPECT_EQ(twice[3], 4);
}

TEST_CASE(join)
{
    Vector<Vector<int>> nested;
    nested.PushBack(Vector<int>{1, 2});
    nested.PushBack(Vector<int>{});
    nested.PushBack(Vector<int>{3});
    nested.PushBack(Vector<int>{4, 5});

    auto flat = Collect(nested | Views::Join);
    EXPECT_EQ(flat.Size(), usize(5));
    for (usize i = 0; i < flat.Size(); ++i)
        EXPECT_EQ(flat[i], static_cast<int>(i) + 1);
}

TEST_CASE(enumerate)
{
    Vector<int> v{10, 20, 30};
    ptrdiff     expected = 0;
    for (auto [index, value] : v | Views::Enumerate)
    {
        EXPECT_EQ(index, expected);
        EXPECT_EQ(value, static_cast<int>(10 * (index + 1)));
        value += 1;
        ++expected;
    }
    EXPECT_EQ(expected, ptrdiff(3));
    // Value is a reference into the range.
    EXPECT_EQ(v[0], 11);
}

TEST_CASE(keys_values)
{
    Vector<KeyValuePair<int, int>> pairs;
    pairs.PushBack(KeyValuePair<int, int>(1, 100));
    pairs.PushBack(KeyValuePair<int, int>(2, 200));

    auto keys   = Collect(pairs | Views::Keys);
    auto values = Collect(pairs | Views::Values);
    EXPECT_EQ(keys[1], 2);
    EXPECT_EQ(values[0], 100);

    for (auto& value : pairs | Views::Values) value = 0;
    EXPECT_EQ(pairs[1].Value, 0);
}

TEST_CASE(single_empty_counted)
{
    auto single = Views::Single(42);
    EXPECT_EQ(single.Size(), usize(1));
    EXPECT_EQ(*single.begin(), 42);

    auto empty = Views::Empty<int>;
    EXPECT(empty.Empty());
    EXPECT_EQ(empty.Size(), usize(0));

    Vector<int> v{1, 2, 3, 4};
    auto        counted = Views::Counted(v.begin() + 1, 2);
    EXPECT_EQ(counted.Size(), usize(2));
    EXPECT_EQ(counted.Front(), 2);
}

TEST_CASE(views_over_string_view)
{
    StringView text = "a1b2c3"_sv;
    usize      digits = 0;
    for (char c : text | Views::Filter([](char ch) { return ch >= '0' && ch <= '9'; }))
    {
        EXPECT(c >= '0' && c <= '9');
        ++digits;
    }
    EXPECT_EQ(digits, usize(3));
}
