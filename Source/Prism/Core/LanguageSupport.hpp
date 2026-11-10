/*
 * Created by v1tr10l7 on 10.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/**
 * Language support library: the few names in namespace std that GCC and Clang
 * look up *by name* while compiling core language constructs.
 *
 *   std::initializer_list            braced-init-lists
 *   std::partial/weak/strong_ordering built-in and defaulted operator<=>
 *   std::tuple_size, tuple_element    structured bindings of tuple-like types
 *   std::coroutine_traits, _handle    co_await / co_yield / co_return
 *   std::source_location::__impl      __builtin_source_location()
 *   std::byte                         may alias anything, provides storage
 *   std::align_val_t                  over-aligned new / delete
 *   std::destroying_delete_t          destroying operator delete
 *   std::construct_at, destroy_at     placement new/destructor in constexpr
 *   std::type_info                    typeid (needs a C++ ABI runtime)
 *   std::exception, bad_alloc, ...    thrown by the ABI runtime
 *
 * They can't live in namespace Prism: the compiler only accepts them in std,
 * with the exact shape it expects (member names and layouts below are the ones
 * GCC and Clang check for, the rest is Prism's own implementation).
 *
 * Every section uses the toolchain's own header when one exists, so a hosted
 * build never sees two definitions of the same std entity. Only when the header
 * is missing (freestanding CryptixOS toolchains) does Prism define the names.
 * Define PRISM_LANGUAGE_SUPPORT_OWN=1 to always use Prism's definitions; then no
 * TU may include the corresponding std headers.
 *
 * Define PRISM_NO_PLACEMENT_NEW=1 if the environment (e.g. the kernel) already
 * defines the placement forms of operator new / delete.
 */

#ifndef PRISM_LANGUAGE_SUPPORT_OWN
    #define PRISM_LANGUAGE_SUPPORT_OWN 0
#endif
#ifndef PRISM_NO_PLACEMENT_NEW
    #define PRISM_NO_PLACEMENT_NEW 0
#endif

#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<initializer_list>)
    #define PRISM_LS_STD_INITIALIZER_LIST 1
#else
    #define PRISM_LS_STD_INITIALIZER_LIST 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<compare>)
    #define PRISM_LS_STD_COMPARE 1
#else
    #define PRISM_LS_STD_COMPARE 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<utility>)
    #define PRISM_LS_STD_UTILITY 1
#else
    #define PRISM_LS_STD_UTILITY 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<cstddef>)
    #define PRISM_LS_STD_CSTDDEF 1
#else
    #define PRISM_LS_STD_CSTDDEF 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<new>)
    #define PRISM_LS_STD_NEW 1
#else
    #define PRISM_LS_STD_NEW 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<memory>)
    #define PRISM_LS_STD_MEMORY 1
#else
    #define PRISM_LS_STD_MEMORY 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<source_location>)
    #define PRISM_LS_STD_SOURCE_LOCATION 1
#else
    #define PRISM_LS_STD_SOURCE_LOCATION 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<coroutine>)
    #define PRISM_LS_STD_COROUTINE 1
#else
    #define PRISM_LS_STD_COROUTINE 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<typeinfo>)
    #define PRISM_LS_STD_TYPEINFO 1
#else
    #define PRISM_LS_STD_TYPEINFO 0
#endif
#if !PRISM_LANGUAGE_SUPPORT_OWN && __has_include(<exception>)
    #define PRISM_LS_STD_EXCEPTION 1
#else
    #define PRISM_LS_STD_EXCEPTION 0
#endif

#if defined(__cpp_constexpr) && __cpp_constexpr >= 202406L
    // C++26: placement new is usable in constant expressions.
    #define PRISM_LS_CONSTEXPR_PLACEMENT_NEW constexpr
#else
    #define PRISM_LS_CONSTEXPR_PLACEMENT_NEW
#endif

// Helpers shared by the sections below; not part of the std interface.
namespace std::__prism
{
    using size_t    = __SIZE_TYPE__;
    using nullptr_t = decltype(nullptr);

    template <typename T>
    T&& declval() noexcept;
} // namespace std::__prism

// ------------------------------------------------------- initializer_list
#if PRISM_LS_STD_INITIALIZER_LIST
    #include <initializer_list> // IWYU pragma: export
#else
namespace std
{
    /**
     * The compiler materializes a const E[N] array and initializes the two
     * members directly: GCC requires exactly {const E*, size_t} in this order,
     * Clang accepts that layout as well.
     */
    template <typename E>
    class initializer_list
    {
      public:
        using value_type      = E;
        using reference       = const E&;
        using const_reference = const E&;
        using size_type       = __prism::size_t;
        using iterator        = const E*;
        using const_iterator  = const E*;

        constexpr initializer_list() noexcept
            : m_Array(nullptr)
            , m_Length(0)
        {
        }

        constexpr size_type      size() const noexcept { return m_Length; }
        constexpr const_iterator begin() const noexcept { return m_Array; }
        constexpr const_iterator end() const noexcept
        {
            return m_Array + m_Length;
        }

      private:
        iterator  m_Array;
        size_type m_Length;

        // Used by the compiler only.
        constexpr initializer_list(const_iterator array, size_type length)
            : m_Array(array)
            , m_Length(length)
        {
        }
    };

    template <typename E>
    constexpr const E* begin(initializer_list<E> list) noexcept
    {
        return list.begin();
    }
    template <typename E>
    constexpr const E* end(initializer_list<E> list) noexcept
    {
        return list.end();
    }
} // namespace std
#endif

// ------------------------------------------------- comparison categories
#if PRISM_LS_STD_COMPARE
    #include <compare> // IWYU pragma: export
#else
namespace std
{
    namespace __prism
    {
        using ComparisonValue = signed char;

        enum class Order : ComparisonValue
        {
            eLess       = -1,
            eEquivalent = 0,
            eGreater    = 1,
        };
        enum class Unordered : ComparisonValue
        {
            eUnordered = 2,
        };

        /**
         * The only argument the relational operators accept is a literal 0:
         * it converts to a null pointer of this type, any other value is
         * rejected at compile time by the consteval constructor.
         */
        struct ZeroLiteral
        {
            consteval ZeroLiteral(ZeroLiteral*) noexcept {}
        };
    } // namespace __prism

    /**
     * Clang and GCC require each category to be a literal class whose only
     * non-static data member is integral, with static constexpr members named
     * less / equivalent / greater (/ equal / unordered) that they evaluate.
     */
    class partial_ordering
    {
        __prism::ComparisonValue m_Value;

        constexpr explicit partial_ordering(__prism::Order value) noexcept
            : m_Value(static_cast<__prism::ComparisonValue>(value))
        {
        }
        constexpr explicit partial_ordering(__prism::Unordered value) noexcept
            : m_Value(static_cast<__prism::ComparisonValue>(value))
        {
        }

        friend class weak_ordering;
        friend class strong_ordering;

      public:
        static const partial_ordering less;
        static const partial_ordering equivalent;
        static const partial_ordering greater;
        static const partial_ordering unordered;

        friend constexpr bool
        operator==(partial_ordering, partial_ordering) noexcept = default;

        [[nodiscard]] friend constexpr bool
        operator==(partial_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value == 0;
        }
        [[nodiscard]] friend constexpr bool
        operator<(partial_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value == -1;
        }
        [[nodiscard]] friend constexpr bool
        operator>(partial_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value == 1;
        }
        [[nodiscard]] friend constexpr bool
        operator<=(partial_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value == -1 || v.m_Value == 0;
        }
        [[nodiscard]] friend constexpr bool
        operator>=(partial_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value == 0 || v.m_Value == 1;
        }
        [[nodiscard]] friend constexpr bool
        operator<(__prism::ZeroLiteral, partial_ordering v) noexcept
        {
            return v.m_Value == 1;
        }
        [[nodiscard]] friend constexpr bool
        operator>(__prism::ZeroLiteral, partial_ordering v) noexcept
        {
            return v.m_Value == -1;
        }
        [[nodiscard]] friend constexpr bool
        operator<=(__prism::ZeroLiteral, partial_ordering v) noexcept
        {
            return v.m_Value == 0 || v.m_Value == 1;
        }
        [[nodiscard]] friend constexpr bool
        operator>=(__prism::ZeroLiteral, partial_ordering v) noexcept
        {
            return v.m_Value == -1 || v.m_Value == 0;
        }
        [[nodiscard]] friend constexpr partial_ordering
        operator<=>(partial_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v;
        }
        [[nodiscard]] friend constexpr partial_ordering
        operator<=>(__prism::ZeroLiteral, partial_ordering v) noexcept
        {
            if (v.m_Value == -1 || v.m_Value == 1)
                return partial_ordering(
                    static_cast<__prism::Order>(-v.m_Value));
            return v;
        }
    };

    inline constexpr partial_ordering
        partial_ordering::less(__prism::Order::eLess);
    inline constexpr partial_ordering
        partial_ordering::equivalent(__prism::Order::eEquivalent);
    inline constexpr partial_ordering
        partial_ordering::greater(__prism::Order::eGreater);
    inline constexpr partial_ordering
        partial_ordering::unordered(__prism::Unordered::eUnordered);

    class weak_ordering
    {
        __prism::ComparisonValue m_Value;

        constexpr explicit weak_ordering(__prism::Order value) noexcept
            : m_Value(static_cast<__prism::ComparisonValue>(value))
        {
        }

        friend class strong_ordering;

      public:
        static const weak_ordering less;
        static const weak_ordering equivalent;
        static const weak_ordering greater;

        [[nodiscard]] constexpr operator partial_ordering() const noexcept
        {
            return partial_ordering(static_cast<__prism::Order>(m_Value));
        }

        friend constexpr bool
        operator==(weak_ordering, weak_ordering) noexcept = default;

        [[nodiscard]] friend constexpr bool
        operator==(weak_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value == 0;
        }
        [[nodiscard]] friend constexpr bool
        operator<(weak_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value < 0;
        }
        [[nodiscard]] friend constexpr bool
        operator>(weak_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value > 0;
        }
        [[nodiscard]] friend constexpr bool
        operator<=(weak_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value <= 0;
        }
        [[nodiscard]] friend constexpr bool
        operator>=(weak_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value >= 0;
        }
        [[nodiscard]] friend constexpr bool
        operator<(__prism::ZeroLiteral, weak_ordering v) noexcept
        {
            return 0 < v.m_Value;
        }
        [[nodiscard]] friend constexpr bool
        operator>(__prism::ZeroLiteral, weak_ordering v) noexcept
        {
            return 0 > v.m_Value;
        }
        [[nodiscard]] friend constexpr bool
        operator<=(__prism::ZeroLiteral, weak_ordering v) noexcept
        {
            return 0 <= v.m_Value;
        }
        [[nodiscard]] friend constexpr bool
        operator>=(__prism::ZeroLiteral, weak_ordering v) noexcept
        {
            return 0 >= v.m_Value;
        }
        [[nodiscard]] friend constexpr weak_ordering
        operator<=>(weak_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v;
        }
        [[nodiscard]] friend constexpr weak_ordering
        operator<=>(__prism::ZeroLiteral, weak_ordering v) noexcept
        {
            return weak_ordering(static_cast<__prism::Order>(-v.m_Value));
        }
    };

    inline constexpr weak_ordering weak_ordering::less(__prism::Order::eLess);
    inline constexpr weak_ordering
        weak_ordering::equivalent(__prism::Order::eEquivalent);
    inline constexpr weak_ordering
        weak_ordering::greater(__prism::Order::eGreater);

    class strong_ordering
    {
        __prism::ComparisonValue m_Value;

        constexpr explicit strong_ordering(__prism::Order value) noexcept
            : m_Value(static_cast<__prism::ComparisonValue>(value))
        {
        }

      public:
        static const strong_ordering less;
        static const strong_ordering equal;
        static const strong_ordering equivalent;
        static const strong_ordering greater;

        [[nodiscard]] constexpr operator partial_ordering() const noexcept
        {
            return partial_ordering(static_cast<__prism::Order>(m_Value));
        }
        [[nodiscard]] constexpr operator weak_ordering() const noexcept
        {
            return weak_ordering(static_cast<__prism::Order>(m_Value));
        }

        friend constexpr bool
        operator==(strong_ordering, strong_ordering) noexcept = default;

        [[nodiscard]] friend constexpr bool
        operator==(strong_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value == 0;
        }
        [[nodiscard]] friend constexpr bool
        operator<(strong_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value < 0;
        }
        [[nodiscard]] friend constexpr bool
        operator>(strong_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value > 0;
        }
        [[nodiscard]] friend constexpr bool
        operator<=(strong_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value <= 0;
        }
        [[nodiscard]] friend constexpr bool
        operator>=(strong_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v.m_Value >= 0;
        }
        [[nodiscard]] friend constexpr bool
        operator<(__prism::ZeroLiteral, strong_ordering v) noexcept
        {
            return 0 < v.m_Value;
        }
        [[nodiscard]] friend constexpr bool
        operator>(__prism::ZeroLiteral, strong_ordering v) noexcept
        {
            return 0 > v.m_Value;
        }
        [[nodiscard]] friend constexpr bool
        operator<=(__prism::ZeroLiteral, strong_ordering v) noexcept
        {
            return 0 <= v.m_Value;
        }
        [[nodiscard]] friend constexpr bool
        operator>=(__prism::ZeroLiteral, strong_ordering v) noexcept
        {
            return 0 >= v.m_Value;
        }
        [[nodiscard]] friend constexpr strong_ordering
        operator<=>(strong_ordering v, __prism::ZeroLiteral) noexcept
        {
            return v;
        }
        [[nodiscard]] friend constexpr strong_ordering
        operator<=>(__prism::ZeroLiteral, strong_ordering v) noexcept
        {
            return strong_ordering(static_cast<__prism::Order>(-v.m_Value));
        }
    };

    inline constexpr strong_ordering
        strong_ordering::less(__prism::Order::eLess);
    inline constexpr strong_ordering
        strong_ordering::equal(__prism::Order::eEquivalent);
    inline constexpr strong_ordering
        strong_ordering::equivalent(__prism::Order::eEquivalent);
    inline constexpr strong_ordering
        strong_ordering::greater(__prism::Order::eGreater);

    [[nodiscard]] constexpr bool is_eq(partial_ordering c) noexcept
    {
        return c == 0;
    }
    [[nodiscard]] constexpr bool is_neq(partial_ordering c) noexcept
    {
        return c != 0;
    }
    [[nodiscard]] constexpr bool is_lt(partial_ordering c) noexcept
    {
        return c < 0;
    }
    [[nodiscard]] constexpr bool is_lteq(partial_ordering c) noexcept
    {
        return c <= 0;
    }
    [[nodiscard]] constexpr bool is_gt(partial_ordering c) noexcept
    {
        return c > 0;
    }
    [[nodiscard]] constexpr bool is_gteq(partial_ordering c) noexcept
    {
        return c >= 0;
    }
} // namespace std
#endif

// ------------------------------------------- tuple protocol (bindings)
#if PRISM_LS_STD_UTILITY
    #include <utility> // IWYU pragma: export
#else
namespace std
{
    /**
     * auto [a, b] = e; treats e as tuple-like iff std::tuple_size<E> is a
     * complete type with a ::value member, so the primary template must stay
     * undefined and the cv-specializations must vanish (by constraint) when
     * the unqualified one doesn't exist, or every const aggregate would break.
     */
    template <typename T>
    struct tuple_size;

    template <typename T>
        requires requires { tuple_size<T>::value; }
    struct tuple_size<const T>
    {
        using value_type                  = __prism::size_t;
        static constexpr value_type value = tuple_size<T>::value;
        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };
    template <typename T>
        requires requires { tuple_size<T>::value; }
    struct tuple_size<volatile T>
    {
        using value_type                  = __prism::size_t;
        static constexpr value_type value = tuple_size<T>::value;
        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };
    template <typename T>
        requires requires { tuple_size<T>::value; }
    struct tuple_size<const volatile T>
    {
        using value_type                  = __prism::size_t;
        static constexpr value_type value = tuple_size<T>::value;
        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };

    template <typename T>
    inline constexpr __prism::size_t tuple_size_v = tuple_size<T>::value;

    template <__prism::size_t I, typename T>
    struct tuple_element;

    template <__prism::size_t I, typename T>
    struct tuple_element<I, const T>
    {
        using type = const typename tuple_element<I, T>::type;
    };
    template <__prism::size_t I, typename T>
    struct tuple_element<I, volatile T>
    {
        using type = volatile typename tuple_element<I, T>::type;
    };
    template <__prism::size_t I, typename T>
    struct tuple_element<I, const volatile T>
    {
        using type = const volatile typename tuple_element<I, T>::type;
    };

    template <__prism::size_t I, typename T>
    using tuple_element_t = typename tuple_element<I, T>::type;
} // namespace std
#endif

// ------------------------------------------------------------------ byte
#if PRISM_LS_STD_CSTDDEF
    #include <cstddef> // IWYU pragma: export
#else
namespace std
{
    /**
     * GCC and Clang recognize std::byte by name: like the character types it
     * may alias any object, and an array of it may provide storage for others.
     */
    enum class byte : unsigned char
    {
    };

    namespace __prism
    {
        template <typename T>
        inline constexpr bool IsByteOperand = false;

        // clang-format off
        template <> inline constexpr bool IsByteOperand<bool>               = true;
        template <> inline constexpr bool IsByteOperand<char>               = true;
        template <> inline constexpr bool IsByteOperand<signed char>        = true;
        template <> inline constexpr bool IsByteOperand<unsigned char>      = true;
        template <> inline constexpr bool IsByteOperand<wchar_t>            = true;
        template <> inline constexpr bool IsByteOperand<char8_t>            = true;
        template <> inline constexpr bool IsByteOperand<char16_t>           = true;
        template <> inline constexpr bool IsByteOperand<char32_t>           = true;
        template <> inline constexpr bool IsByteOperand<short>              = true;
        template <> inline constexpr bool IsByteOperand<unsigned short>     = true;
        template <> inline constexpr bool IsByteOperand<int>                = true;
        template <> inline constexpr bool IsByteOperand<unsigned int>       = true;
        template <> inline constexpr bool IsByteOperand<long>               = true;
        template <> inline constexpr bool IsByteOperand<unsigned long>      = true;
        template <> inline constexpr bool IsByteOperand<long long>          = true;
        template <> inline constexpr bool IsByteOperand<unsigned long long> = true;
    #if defined(__SIZEOF_INT128__)
        template <> inline constexpr bool IsByteOperand<__int128>           = true;
        template <> inline constexpr bool IsByteOperand<unsigned __int128>  = true;
    #endif
        // clang-format on

        template <typename T>
        inline constexpr bool IsByteOperand<const T> = IsByteOperand<T>;
        template <typename T>
        inline constexpr bool IsByteOperand<volatile T> = IsByteOperand<T>;
        template <typename T>
        inline constexpr bool IsByteOperand<const volatile T>
            = IsByteOperand<T>;
    } // namespace __prism

    template <typename I>
        requires __prism::IsByteOperand<I>
    [[nodiscard]] constexpr byte operator<<(byte b, I shift) noexcept
    {
        return static_cast<byte>(
            static_cast<unsigned char>(static_cast<unsigned int>(b) << shift));
    }
    template <typename I>
        requires __prism::IsByteOperand<I>
    [[nodiscard]] constexpr byte operator>>(byte b, I shift) noexcept
    {
        return static_cast<byte>(
            static_cast<unsigned char>(static_cast<unsigned int>(b) >> shift));
    }
    template <typename I>
        requires __prism::IsByteOperand<I>
    constexpr byte& operator<<=(byte& b, I shift) noexcept
    {
        return b = b << shift;
    }
    template <typename I>
        requires __prism::IsByteOperand<I>
    constexpr byte& operator>>=(byte& b, I shift) noexcept
    {
        return b = b >> shift;
    }

    [[nodiscard]] constexpr byte operator|(byte l, byte r) noexcept
    {
        return static_cast<byte>(static_cast<unsigned char>(
            static_cast<unsigned int>(l) | static_cast<unsigned int>(r)));
    }
    [[nodiscard]] constexpr byte operator&(byte l, byte r) noexcept
    {
        return static_cast<byte>(static_cast<unsigned char>(
            static_cast<unsigned int>(l) & static_cast<unsigned int>(r)));
    }
    [[nodiscard]] constexpr byte operator^(byte l, byte r) noexcept
    {
        return static_cast<byte>(static_cast<unsigned char>(
            static_cast<unsigned int>(l) ^ static_cast<unsigned int>(r)));
    }
    [[nodiscard]] constexpr byte operator~(byte b) noexcept
    {
        return static_cast<byte>(
            static_cast<unsigned char>(~static_cast<unsigned int>(b)));
    }
    constexpr byte& operator|=(byte& l, byte r) noexcept { return l = l | r; }
    constexpr byte& operator&=(byte& l, byte r) noexcept { return l = l & r; }
    constexpr byte& operator^=(byte& l, byte r) noexcept { return l = l ^ r; }

    template <typename I>
        requires __prism::IsByteOperand<I>
    [[nodiscard]] constexpr I to_integer(byte b) noexcept
    {
        return static_cast<I>(b);
    }
} // namespace std
#endif

// ------------------------------------------------------------- new/delete
#if PRISM_LS_STD_NEW
    #include <new> // IWYU pragma: export
#else
namespace std
{
    /**
     * Both compilers declare std::align_val_t implicitly for the aligned
     * global operator new / delete; an explicit declaration must match it.
     */
    enum class align_val_t : __prism::size_t
    {
    };

    struct nothrow_t
    {
        explicit nothrow_t() = default;
    };
    /**
     * libstdc++ and libc++ declare this extern and define it in their runtime;
     * there is no such runtime here, so it's an inline variable instead.
     */
    inline constexpr nothrow_t nothrow{};

    /// The compiler checks a destroying delete's 2nd parameter for this type.
    struct destroying_delete_t
    {
        explicit destroying_delete_t() = default;
    };
    inline constexpr destroying_delete_t destroying_delete{};

    template <typename T>
    [[nodiscard]] constexpr T* launder(T* pointer) noexcept
    {
        return __builtin_launder(pointer);
    }
    // Laundering functions or void pointers is ill-formed.
    template <typename R, typename... Args, bool NoExcept>
    void launder(R (*)(Args...) noexcept(NoExcept)) = delete;
    template <typename R, typename... Args, bool NoExcept>
    void launder(R (*)(Args..., ...) noexcept(NoExcept)) = delete;
    void launder(void*)                                  = delete;
    void launder(const void*)                            = delete;
    void launder(volatile void*)                         = delete;
    void launder(const volatile void*)                   = delete;
} // namespace std

// The replaceable global allocation functions. The compiler declares these
// implicitly in every TU; the environment (kernel / libc) provides them.
[[nodiscard]] void* operator new(__SIZE_TYPE__ size);
[[nodiscard]] void* operator new[](__SIZE_TYPE__ size);
[[nodiscard]] void* operator new(__SIZE_TYPE__ size,
                                 const std::nothrow_t&) noexcept;
[[nodiscard]] void* operator new[](__SIZE_TYPE__ size,
                                   const std::nothrow_t&) noexcept;
[[nodiscard]] void* operator new(__SIZE_TYPE__ size, std::align_val_t align);
[[nodiscard]] void* operator new[](__SIZE_TYPE__ size, std::align_val_t align);
[[nodiscard]] void* operator new(__SIZE_TYPE__ size, std::align_val_t align,
                                 const std::nothrow_t&) noexcept;
[[nodiscard]] void* operator new[](__SIZE_TYPE__ size, std::align_val_t align,
                                   const std::nothrow_t&) noexcept;

void operator delete(void* pointer) noexcept;
void operator delete[](void* pointer) noexcept;
void operator delete(void* pointer, __SIZE_TYPE__ size) noexcept;
void operator delete[](void* pointer, __SIZE_TYPE__ size) noexcept;
void operator delete(void* pointer, const std::nothrow_t&) noexcept;
void operator delete[](void* pointer, const std::nothrow_t&) noexcept;
void operator delete(void* pointer, std::align_val_t align) noexcept;
void operator delete[](void* pointer, std::align_val_t align) noexcept;
void operator delete(void* pointer, __SIZE_TYPE__ size,
                     std::align_val_t align) noexcept;
void operator delete[](void* pointer, __SIZE_TYPE__ size,
                       std::align_val_t align) noexcept;
void operator delete(void* pointer, std::align_val_t align,
                     const std::nothrow_t&) noexcept;
void operator delete[](void* pointer, std::align_val_t align,
                       const std::nothrow_t&) noexcept;

    #if !PRISM_NO_PLACEMENT_NEW
[[nodiscard]] inline PRISM_LS_CONSTEXPR_PLACEMENT_NEW void*
operator new(__SIZE_TYPE__, void* pointer) noexcept
{
    return pointer;
}
[[nodiscard]] inline PRISM_LS_CONSTEXPR_PLACEMENT_NEW void*
operator new[](__SIZE_TYPE__, void* pointer) noexcept
{
    return pointer;
}
inline void operator delete(void*, void*) noexcept {}
inline void operator delete[](void*, void*) noexcept {}
    #endif
#endif

// ------------------------------------------------ construct_at/destroy_at
#if PRISM_LS_STD_MEMORY
    #include <memory> // IWYU pragma: export
#else
namespace std
{
    /**
     * Before C++26, placement new is not a constant expression, but GCC and
     * Clang allow a call to std::construct_at (recognized by name) during
     * constant evaluation, which is how constexpr containers construct objects.
     */
    template <typename T, typename... Args>
    constexpr auto construct_at(T* location, Args&&... args) noexcept(
        noexcept(::new(static_cast<void*>(nullptr))
                     T(__prism::declval<Args>()...)))
        -> decltype(::new(static_cast<void*>(nullptr))
                        T(__prism::declval<Args>()...))
    {
        return ::new (static_cast<void*>(location))
            T(static_cast<Args&&>(args)...);
    }

    template <typename T>
    constexpr void destroy_at(T* location)
    {
        if constexpr (__is_array(T))
        {
            for (auto& element : *location)
                std::destroy_at(__builtin_addressof(element));
        }
        else location->~T();
    }
} // namespace std
#endif

// ------------------------------------------------------- source_location
#if PRISM_LS_STD_SOURCE_LOCATION
    #include <source_location> // IWYU pragma: export
#elif defined(__has_builtin) && __has_builtin(__builtin_source_location)
namespace std
{
    struct source_location
    {
      private:
        /**
         * Both compilers fill this struct and check its name and members:
         * exactly these four, with these names and types. It has to be
         * complete before __builtin_source_location() is first named.
         */
        struct __impl
        {
            const char* _M_file_name;
            const char* _M_function_name;
            unsigned    _M_line;
            unsigned    _M_column;
        };

        // GCC's builtin returns const void*, Clang's returns const __impl*.
        using BuiltinType = decltype(__builtin_source_location());

      public:
        static consteval source_location
        current(BuiltinType location = __builtin_source_location()) noexcept
        {
            source_location result;
            result.m_Impl = static_cast<const __impl*>(location);
            return result;
        }

        constexpr source_location() noexcept {}

        constexpr __UINT_LEAST32_TYPE__ line() const noexcept
        {
            return m_Impl ? m_Impl->_M_line : 0u;
        }
        constexpr __UINT_LEAST32_TYPE__ column() const noexcept
        {
            return m_Impl ? m_Impl->_M_column : 0u;
        }
        constexpr const char* file_name() const noexcept
        {
            return m_Impl ? m_Impl->_M_file_name : "";
        }
        constexpr const char* function_name() const noexcept
        {
            return m_Impl ? m_Impl->_M_function_name : "";
        }

      private:
        const __impl* m_Impl = nullptr;
    };
} // namespace std
#endif

// ------------------------------------------------------------ coroutines
#if PRISM_LS_STD_COROUTINE
    #include <coroutine> // IWYU pragma: export
#elif defined(__cpp_impl_coroutine)
namespace std
{
    namespace __prism
    {
        template <typename R, typename = void>
        struct CoroutineTraitsBase
        {
        };
        template <typename R>
            requires requires { typename R::promise_type; }
        struct CoroutineTraitsBase<R>
        {
            using promise_type = typename R::promise_type;
        };
    } // namespace __prism

    /// The compiler finds the promise type through
    /// std::coroutine_traits<Return, Params...>::promise_type.
    template <typename R, typename... Args>
    struct coroutine_traits : __prism::CoroutineTraitsBase<R>
    {
    };

    template <typename Promise = void>
    struct coroutine_handle;

    /**
     * The compiler calls coroutine_handle<P>::from_address() on the frame
     * pointer and converts handles to coroutine_handle<> for await_suspend.
     */
    template <>
    struct coroutine_handle<void>
    {
        constexpr coroutine_handle() noexcept = default;
        constexpr coroutine_handle(__prism::nullptr_t) noexcept {}

        coroutine_handle& operator=(__prism::nullptr_t) noexcept
        {
            m_Frame = nullptr;
            return *this;
        }

        constexpr void* address() const noexcept { return m_Frame; }
        static constexpr coroutine_handle from_address(void* frame) noexcept
        {
            coroutine_handle handle;
            handle.m_Frame = frame;
            return handle;
        }

        constexpr explicit operator bool() const noexcept
        {
            return m_Frame != nullptr;
        }

        bool done() const noexcept { return __builtin_coro_done(m_Frame); }
        void operator()() const { resume(); }
        void resume() const { __builtin_coro_resume(m_Frame); }
        void destroy() const { __builtin_coro_destroy(m_Frame); }

      private:
        void* m_Frame = nullptr;
    };

    template <typename Promise>
    struct coroutine_handle
    {
        constexpr coroutine_handle() noexcept = default;
        constexpr coroutine_handle(__prism::nullptr_t) noexcept {}

        static coroutine_handle from_promise(Promise& promise)
        {
            coroutine_handle handle;
            handle.m_Frame = __builtin_coro_promise(
                reinterpret_cast<char*>(__builtin_addressof(promise)),
                alignof(Promise), true);
            return handle;
        }

        coroutine_handle& operator=(__prism::nullptr_t) noexcept
        {
            m_Frame = nullptr;
            return *this;
        }

        constexpr void* address() const noexcept { return m_Frame; }
        static constexpr coroutine_handle from_address(void* frame) noexcept
        {
            coroutine_handle handle;
            handle.m_Frame = frame;
            return handle;
        }

        constexpr operator coroutine_handle<>() const noexcept
        {
            return coroutine_handle<>::from_address(m_Frame);
        }

        constexpr explicit operator bool() const noexcept
        {
            return m_Frame != nullptr;
        }

        bool done() const noexcept { return __builtin_coro_done(m_Frame); }
        void operator()() const { resume(); }
        void resume() const { __builtin_coro_resume(m_Frame); }
        void destroy() const { __builtin_coro_destroy(m_Frame); }

        Promise& promise() const
        {
            return *static_cast<Promise*>(
                __builtin_coro_promise(m_Frame, alignof(Promise), false));
        }

      private:
        void* m_Frame = nullptr;
    };

    constexpr bool operator==(coroutine_handle<> lhs,
                              coroutine_handle<> rhs) noexcept
    {
        return lhs.address() == rhs.address();
    }
    constexpr strong_ordering operator<=>(coroutine_handle<> lhs,
                                          coroutine_handle<> rhs) noexcept
    {
        return lhs.address() <=> rhs.address();
    }

    struct suspend_always
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_suspend(coroutine_handle<>) const noexcept {}
        constexpr void await_resume() const noexcept {}
    };
    struct suspend_never
    {
        constexpr bool await_ready() const noexcept { return true; }
        constexpr void await_suspend(coroutine_handle<>) const noexcept {}
        constexpr void await_resume() const noexcept {}
    };
} // namespace std
#endif

// ----------------------------------------------------- type_info (RTTI)
#if PRISM_LS_STD_TYPEINFO
    #include <typeinfo> // IWYU pragma: export
#elif defined(__GXX_RTTI) || defined(__cpp_rtti)
namespace __cxxabiv1
{
    class __class_type_info;
} // namespace __cxxabiv1

namespace std
{
    /**
     * typeid yields objects whose layout and vtable come from the Itanium C++
     * ABI runtime (libsupc++ / libcxxrt): it defines ~type_info (the key
     * function, so the vtable), before(), hash_code() and the __ hooks. This
     * declaration must match that runtime's layout; without one, keep RTTI
     * disabled (-fno-rtti) and this section compiles out.
     */
    class type_info
    {
      public:
        virtual ~type_info();

        const char* name() const noexcept
        {
            return m_Name[0] == '*' ? m_Name + 1 : m_Name;
        }

        bool before(const type_info& other) const noexcept;
        __prism::size_t hash_code() const noexcept;

        bool operator==(const type_info& other) const noexcept
        {
            if (m_Name == other.m_Name) return true;
            // A leading '*' marks names that are unique per type.
            return m_Name[0] != '*'
                && __builtin_strcmp(m_Name, other.m_Name) == 0;
        }

        virtual bool __is_pointer_p() const;
        virtual bool __is_function_p() const;
        virtual bool __do_catch(const type_info* thrownType, void** thrownObject,
                                unsigned outer) const;
        virtual bool __do_upcast(const __cxxabiv1::__class_type_info* target,
                                 void** object) const;

      protected:
        const char* m_Name;

        explicit type_info(const char* name)
            : m_Name(name)
        {
        }

      private:
        type_info(const type_info&)            = delete;
        type_info& operator=(const type_info&) = delete;
    };
} // namespace std
#endif

// ------------------------------------------------------------ exceptions
#if PRISM_LS_STD_EXCEPTION
    #include <exception> // IWYU pragma: export
#elif defined(__cpp_exceptions)
namespace std
{
    /**
     * Thrown by the ABI runtime (e.g. bad_array_new_length from new T[n] with
     * a bad n), so the vtables and what() come from that runtime as well.
     */
    class exception
    {
      public:
        exception() noexcept {}
        exception(const exception&)            = default;
        exception& operator=(const exception&) = default;
        virtual ~exception() noexcept;
        virtual const char* what() const noexcept;
    };

    class bad_alloc : public exception
    {
      public:
        bad_alloc() noexcept {}
        bad_alloc(const bad_alloc&)            = default;
        bad_alloc& operator=(const bad_alloc&) = default;
        ~bad_alloc() noexcept override;
        const char* what() const noexcept override;
    };

    class bad_array_new_length : public bad_alloc
    {
      public:
        bad_array_new_length() noexcept {}
        ~bad_array_new_length() noexcept override;
        const char* what() const noexcept override;
    };
} // namespace std
#endif
