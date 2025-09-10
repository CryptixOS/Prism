/*
 * Created by v1tr10l7 on 01.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

namespace Prism
{
    template <typename T = void>
    struct Less
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs < rhs;
        }
    };

    template <>
    struct Less<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) < static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct Greater
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs > rhs;
        }
    };

    template <>
    struct Greater<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) > static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct EqualTo
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs == rhs;
        }
    };

    template <>
    struct EqualTo<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) == static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct NotEqualTo
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs != rhs;
        }
    };

    template <>
    struct NotEqualTo<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) != static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct GreaterEqual
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs >= rhs;
        }
    };

    template <>
    struct GreaterEqual<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) >= static_cast<U&&>(rhs);
        }
    };

    template <typename T = void>
    struct LessEqual
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs <= rhs;
        }
    };

    template <>
    struct LessEqual<void>
    {
        template <typename T, typename U>
        constexpr bool operator()(T&& lhs, U&& rhs) const
        {
            return static_cast<T&&>(lhs) <= static_cast<U&&>(rhs);
        }
    };

    struct IteratorLess
    {
        template <typename It1, typename It2>
        constexpr bool operator()(It1 it1, It2 it2) const
        {
            return *it1 < *it2;
        }
    };
    struct IteratorLessValue
    {
        constexpr IteratorLessValue() = default;

        constexpr explicit IteratorLessValue(IteratorLess) {}

        template <typename It, typename T>
        constexpr bool operator()(It it, T& value) const
        {
            return *it < value;
        }
    };
    struct ValueLessIterator
    {
        constexpr ValueLessIterator() = default;

        constexpr explicit ValueLessIterator(IteratorLess) {}

        template <typename T, typename It>
        constexpr bool operator()(T& value, It it) const
        {
            return value < *it;
        }
    };
    struct IteratorEqualToEqualTo
    {
        template <typename It1, typename It2>
        constexpr bool operator()(It1 it1, It2 it2) const
        {
            return *it1 == *it2;
        }
    };
    struct IteratorEqualToValue
    {
        template <typename It, typename T>
        constexpr bool operator()(It it, T& value) const
        {
            return *it == value;
        }
    };
    template <typename Comparator>
    struct IteratorCompareIterator
    {
        Comparator Compare;

        explicit constexpr IteratorCompareIterator(Comparator comparator)
            : Compare(Move(comparator))
        {
        }

        template <typename It1, typename It2>
        constexpr bool operator()(It1 it1, It2 it2)
        {
            return bool(Compare(*it1, *it2));
        }
    };
    template <typename Comparator>
    struct IteratorCompareValue
    {
        Comparator Compare;

        constexpr explicit IteratorCompareValue(Comparator comparator)
            : Compare(Move(comparator))
        {
        }

        constexpr explicit IteratorCompareValue(
            const IteratorCompareIterator<Comparator>& comparator)
            : Compare(comparator.Compare)
        {
        }

        constexpr explicit IteratorCompareValue(
            IteratorCompareIterator<Comparator>&& comparator)
            : Compare(Move(comparator.Compare))
        {
        }

        template <typename It, typename T>
        constexpr bool operator()(It it, T& value)
        {
            return bool(Compare(*it, value));
        }
    };
    template <typename Comparator>
    struct ValueCompareIterator
    {
        Comparator Compare;

        constexpr explicit ValueCompareIterator(Comparator comparator)
            : Compare(Move(comparator))
        {
        }

        constexpr explicit ValueCompareIterator(
            const IteratorCompareIterator<Comparator>& comparator)
            : Compare(comparator.Compare)
        {
        }

        constexpr explicit ValueCompareIterator(
            IteratorCompareIterator<Comparator>&& comparator)
            : Compare(Move(comparator.Compare))
        {
        }

        template <typename T, typename It>
        constexpr bool operator()(T& value, It it)
        {
            return bool(Compare(value, *it));
        }
    };
    template <typename T>
    struct IteratorEqualsValue
    {
        T& Value;

        constexpr explicit IteratorEqualsValue(T& value)
            : Value(value)
        {
        }

        template <typename It>
        constexpr bool operator()(It it)
        {
            return *it == Value;
        }
    };
    template <typename It1>
    struct IteratorEqualsIterator
    {
        It1 Iterator1;

        constexpr explicit IteratorEqualsIterator(It1 it1)
            : Iterator1(it1)
        {
        }

        template <typename It2>
        constexpr bool operator()(It2 it2)
        {
            return *it2 == *Iterator1;
        }
    };
    template <typename Predicate>
    struct IteratorPredicate
    {
        Predicate Compare;

        constexpr explicit IteratorPredicate(Predicate compare)
            : Compare(Move(compare))
        {
        }

        template <typename It>
        constexpr bool operator()(It it)
        {
            return bool(Compare(*it));
        }
    };
    template <typename Comparator, typename T>
    struct IteratorCompareToValue
    {
        Comparator Compare;
        T&         Value;

        constexpr IteratorCompareToValue(Comparator comparator, T& value)
            : Compare(Move(comparator))
            , Value(value)
        {
        }

        template <typename It>
        constexpr bool operator()(It it)
        {
            return bool(Compare(*it, Value));
        }
    };
    template <typename Comparator, typename It1>
    struct IteratorCompareToIterator
    {
        Comparator Compare;
        It1        Iterator1;

        constexpr IteratorCompareToIterator(Comparator comparator, It1 it1)
            : Compare(Move(comparator))
            , Iterator1(it1)
        {
        }

        template <typename It2>
        constexpr bool operator()(It2 it2)
        {
            return bool(Compare(*it2, *Iterator1));
        }
    };
    template <typename Predicate>
    struct IteratorNegate
    {
        Predicate Compare;

        constexpr explicit IteratorNegate(Predicate compare)
            : Compare(Move(compare))
        {
        }

        template <typename It>
        constexpr bool operator()(It it)
        {
            return !bool(Compare(*it));
        }
    };

    constexpr inline IteratorLess IteratorLessType() { return IteratorLess(); }
    constexpr inline IteratorLessValue IteratorLessValueType()
    {
        return IteratorLessValue();
    }
    constexpr inline IteratorLessValue IteratorLessValueType(IteratorLess)
    {
        return IteratorLessValue();
    }
    constexpr inline ValueLessIterator ValueLessIteratorType()
    {
        return ValueLessIterator();
    }
    constexpr inline ValueLessIterator ValueCompareIteratorType(IteratorLess)
    {
        return ValueLessIterator();
    }
    constexpr inline IteratorEqualToEqualTo IteratorEqualToIteratorType()
    {
        return IteratorEqualToEqualTo();
    }
    constexpr inline IteratorEqualToValue IteratorEqualToValueType()
    {
        return IteratorEqualToValue();
    }
    constexpr inline IteratorEqualToValue
    IteratorEqualToValueType(IteratorEqualToEqualTo)
    {
        return IteratorEqualToValue();
    }
    template <typename Comparator>
    constexpr inline IteratorCompareIterator<Comparator>
    IteratorCompareIteratorType(Comparator comparator)
    {
        return IteratorCompareIterator<Comparator>(Move(comparator));
    }
    template <typename Comparator>
    constexpr inline IteratorCompareValue<Comparator>
    IteratorLessValueType(Comparator comparator)
    {
        return IteratorCompareValue<Comparator>(Move(comparator));
    }
    template <typename Comparator>
    constexpr inline IteratorCompareValue<Comparator>
    IteratorLessValueType(IteratorCompareIterator<Comparator> comparator)
    {
        return IteratorCompareValue<Comparator>(Move(comparator));
    }
    template <typename Comparator>
    constexpr inline ValueCompareIterator<Comparator>
    ValueCompareIteratorType(Comparator comparator)
    {
        return ValueCompareIterator<Comparator>(Move(comparator));
    }
    template <typename Comparator>
    constexpr inline ValueCompareIterator<Comparator>
    ValueCompareIteratorType(IteratorCompareIterator<Comparator> comparator)
    {
        return ValueCompareIterator<Comparator>(Move(comparator));
    }
    template <typename T>
    constexpr inline IteratorEqualsValue<T> IteratorEqualsValueType(T& value)
    {
        return IteratorEqualsValue<T>(value);
    }
    template <typename It>
    constexpr inline IteratorEqualsIterator<It>
    IteratorEqualsIteratorType(IteratorEqualToEqualTo, It it)
    {
        return IteratorEqualsIterator<It>(it);
    }
    template <typename Predicate>
    constexpr inline IteratorPredicate<Predicate>
    PredicateIteratorType(Predicate compare)
    {
        return IteratorPredicate<Predicate>(Move(compare));
    }
    template <typename Comparator, typename T>
    IteratorCompareToValue<Comparator, T> constexpr IteratorLessValueType(
        Comparator comparator, T& value)
    {
        return IteratorCompareToValue<Comparator, T>(Move(comparator), value);
    }
    template <typename Comparator, typename It>
    constexpr inline IteratorCompareToIterator<Comparator, It>
    IteratorCompareIteratorType(IteratorCompareIterator<Comparator> comparator,
                                It                                  it)
    {
        return IteratorCompareToIterator<Comparator, It>(
            Move(comparator.Compare), it);
    }
    template <typename Predicate>
    constexpr inline IteratorNegate<Predicate>
    NegateType(IteratorPredicate<Predicate> compare)
    {
        return IteratorNegate<Predicate>(Move(compare.Compare));
    }
} // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::EqualTo;
using Prism::Greater;
using Prism::GreaterEqual;
using Prism::IteratorCompareIterator;
using Prism::IteratorCompareToIterator;
using Prism::IteratorCompareToValue;
using Prism::IteratorCompareValue;
using Prism::IteratorEqualsIterator;
using Prism::IteratorEqualsValue;
using Prism::IteratorEqualToEqualTo;
using Prism::IteratorEqualToValue;
using Prism::IteratorLess;
using Prism::IteratorLessValue;
using Prism::IteratorNegate;
using Prism::IteratorPredicate;
using Prism::Less;
using Prism::LessEqual;
using Prism::NegateType;
using Prism::NotEqualTo;
using Prism::ValueCompareIterator;
#endif
