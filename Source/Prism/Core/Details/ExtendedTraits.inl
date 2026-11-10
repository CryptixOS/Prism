/*
 * Created by v1tr10l7 on 06.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Details/CallableTraits.inl>
#include <Prism/Core/Details/Constructibility.inl>

/**
 * Traits that are not part of <type_traits>, but which libc++ uses
 * internally, or which SerenityOS' AK provides, and are generally useful
 * when writing containers and algorithms.
 */
namespace Prism
{
    // -------------------------------------------------------------- relations

    /// T is the same type as one of Ts (AK::IsOneOf).
    template <typename T, typename... Ts>
    struct IsOneOf : Disjunction<IsSame<T, Ts>...>
    {
    };
    /// T and U are the same type, ignoring top-level cv-qualifiers.
    template <typename T, typename U>
    struct IsSameIgnoringCv : IsSame<RemoveCvType<T>, RemoveCvType<U>>
    {
    };
    /// T is one of Ts, ignoring top-level cv-qualifiers.
    template <typename T, typename... Ts>
    struct IsOneOfIgnoringCv : Disjunction<IsSameIgnoringCv<T, Ts>...>
    {
    };

    namespace Details
    {
        template <template <typename...> typename Base, typename... Args>
        TrueType TestTemplateBase(const volatile Base<Args...>*);
        template <template <typename...> typename>
        FalseType TestTemplateBase(const volatile void*);

        template <template <typename...> typename Base, typename Derived>
        auto TestIsTemplateBaseOf(int)
            -> decltype(TestTemplateBase<Base>(DeclVal<Derived*>()));
        template <template <typename...> typename, typename>
        auto TestIsTemplateBaseOf(...) -> FalseType;
    } // namespace Details

    /// Derived is, or publicly and unambiguously derives from, some
    /// specialization of the class template Base (AK::IsTemplateBaseOf).
    template <template <typename...> typename Base, typename Derived>
    struct IsTemplateBaseOf
        : decltype(Details::TestIsTemplateBaseOf<Base, RemoveCvType<Derived>>(
              0))
    {
    };

    // ---------------------------------------------------- type classification

    /// One of the character types: char, wchar_t, char8_t, char16_t, char32_t.
    template <typename T>
    struct IsCharacter
        : IsOneOf<RemoveCvType<T>, char, wchar_t, char8_t, char16_t, char32_t>
    {
    };
    /// A standard or extended signed integer type: excludes bool and the
    /// character types (libc++'s __libcpp_signed_integer).
    template <typename T>
    struct IsStandardSignedInteger
        : IsOneOf<RemoveCvType<T>, signed char, short, int, long, long long
#ifdef __SIZEOF_INT128__
                  ,
                  __int128
#endif
                  >
    {
    };
    /// A standard or extended unsigned integer type: excludes bool and the
    /// character types (libc++'s __libcpp_unsigned_integer).
    template <typename T>
    struct IsStandardUnsignedInteger
        : IsOneOf<RemoveCvType<T>, unsigned char, unsigned short, unsigned int,
                  unsigned long, unsigned long long
#ifdef __SIZEOF_INT128__
                  ,
                  unsigned __int128
#endif
                  >
    {
    };
    template <typename T>
    struct IsStandardInteger
        : Disjunction<IsStandardSignedInteger<T>, IsStandardUnsignedInteger<T>>
    {
    };

    /// `T&` is a valid type: T is an object type, a function type without
    /// cv/ref qualifiers, or a reference.
    template <typename T>
    struct IsReferenceable : BooleanConstant<requires { typename TypeIdentity<T&>; }>
    {
    };

    // ------------------------------------------------------- transformations

    /// U with the const-qualification of T (AK::CopyConst).
    template <typename T, typename U>
    struct CopyConst
    {
        using Type = ConditionalType<IsConstV<RemoveReferenceType<T>>,
                                     AddConstType<U>, U>;
    };
    template <typename T, typename U>
    using CopyConstType = typename CopyConst<T, U>::Type;

    /// U with the cv-qualifiers of T (libc++'s __copy_cv).
    template <typename T, typename U>
    struct CopyCvQualifiers
    {
        using Type = CopyCv<T, U>;
    };
    template <typename T, typename U>
    using CopyCvType = typename CopyCvQualifiers<T, U>::Type;

    /// U with the cv- and reference-qualifiers of T (libc++'s __copy_cvref).
    template <typename T, typename U>
    struct CopyCvRef
    {
        using Type = CopyCv<T, U>;
    };
    template <typename T, typename U>
    struct CopyCvRef<T&, U>
    {
        using Type = AddLValueReferenceType<CopyCv<T, U>>;
    };
    template <typename T, typename U>
    struct CopyCvRef<T&&, U>
    {
        using Type = AddRValueReferenceType<CopyCv<T, U>>;
    };
    template <typename T, typename U>
    using CopyCvRefType = typename CopyCvRef<T, U>::Type;

    // ------------------------------------------------------- object behaviour

    /**
     * Objects of type T can be relocated (moved to a new address, ending the
     * lifetime of the source) with a plain memcpy.
     *
     * Types can opt in by declaring `using TriviallyRelocatable = TrueType;`
     * or by specializing this trait.
     */
    template <typename T, typename = void>
    struct IsTriviallyRelocatable
#if PrismHasBuiltin(__builtin_is_cpp_trivially_relocatable)
        : BooleanConstant<__builtin_is_cpp_trivially_relocatable(T)>
#else
        : BooleanConstant<__is_trivially_copyable(T)
                          && IsTriviallyDestructible<T>::Value>
#endif
    {
    };
    template <typename T>
    struct IsTriviallyRelocatable<T, VoidType<typename T::TriviallyRelocatable>>
        : BooleanConstant<T::TriviallyRelocatable::Value>
    {
    };

    /**
     * operator== on T and U compares the object representations, so it can
     * be replaced by a memcmp (libc++'s __libcpp_is_trivially_equality_comparable).
     */
    template <typename T, typename U = T>
    struct IsTriviallyEqualityComparable
#if PrismHasBuiltin(__is_trivially_equality_comparable)
        : BooleanConstant<IsSameIgnoringCv<T, U>::Value
                          && __is_trivially_equality_comparable(
                              RemoveCvType<T>)>
#else
        : BooleanConstant<IsSameIgnoringCv<T, U>::Value
                          && (IsIntegral<T>::Value || IsEnum<T>::Value
                              || IsPointer<T>::Value)>
#endif
    {
    };

    // ------------------------------------------------------------- detection

    /// Placeholder type for failed detection; can't be created or copied.
    struct Nonesuch
    {
        ~Nonesuch()                     = delete;
        Nonesuch(const Nonesuch&)       = delete;
        void operator=(const Nonesuch&) = delete;
    };

    template <typename Default, template <typename...> typename Op,
              typename... Args>
    struct DetectedOr
    {
        using Type       = Default;
        using IsDetected = FalseType;
    };
    template <typename Default, template <typename...> typename Op,
              typename... Args>
        requires requires { typename Op<Args...>; }
    struct DetectedOr<Default, Op, Args...>
    {
        using Type       = Op<Args...>;
        using IsDetected = TrueType;
    };

    /// Op<Args...> if it is well-formed, Default otherwise.
    template <typename Default, template <typename...> typename Op,
              typename... Args>
    using DetectedOrType = typename DetectedOr<Default, Op, Args...>::Type;
    /// Op<Args...> if it is well-formed, Nonesuch otherwise.
    template <template <typename...> typename Op, typename... Args>
    using DetectedType = DetectedOrType<Nonesuch, Op, Args...>;
    /// Op<Args...> is well-formed.
    template <template <typename...> typename Op, typename... Args>
    using IsDetected = typename DetectedOr<Nonesuch, Op, Args...>::IsDetected;
    /// Op<Args...> is well-formed and is exactly Expected.
    template <typename Expected, template <typename...> typename Op,
              typename... Args>
    using IsDetectedExact = IsSame<Expected, DetectedType<Op, Args...>>;
    /// Op<Args...> is well-formed and converts to To.
    template <typename To, template <typename...> typename Op,
              typename... Args>
    using IsDetectedConvertible = IsConvertible<DetectedType<Op, Args...>, To>;

    // ------------------------------------------------------------- utilities

    /// Always false, but dependent on Ts: lets `static_assert(DependentFalse<T>)`
    /// fire only when the enclosing template is instantiated.
    template <typename... Ts>
    inline constexpr bool DependentFalse = false;

    // --------------------------------------------------------- V variables

    template <typename T, typename... Ts>
    inline constexpr bool IsOneOfV = IsOneOf<T, Ts...>::Value;
    template <typename T, typename U>
    inline constexpr bool IsSameIgnoringCvV = IsSameIgnoringCv<T, U>::Value;
    template <typename T, typename... Ts>
    inline constexpr bool IsOneOfIgnoringCvV
        = IsOneOfIgnoringCv<T, Ts...>::Value;
    template <template <typename...> typename Base, typename Derived>
    inline constexpr bool IsTemplateBaseOfV
        = IsTemplateBaseOf<Base, Derived>::Value;
    template <typename T>
    inline constexpr bool IsCharacterV = IsCharacter<T>::Value;
    template <typename T>
    inline constexpr bool IsStandardSignedIntegerV
        = IsStandardSignedInteger<T>::Value;
    template <typename T>
    inline constexpr bool IsStandardUnsignedIntegerV
        = IsStandardUnsignedInteger<T>::Value;
    template <typename T>
    inline constexpr bool IsStandardIntegerV = IsStandardInteger<T>::Value;
    template <typename T>
    inline constexpr bool IsReferenceableV = IsReferenceable<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyRelocatableV
        = IsTriviallyRelocatable<T>::Value;
    template <typename T, typename U = T>
    inline constexpr bool IsTriviallyEqualityComparableV
        = IsTriviallyEqualityComparable<T, U>::Value;
    template <template <typename...> typename Op, typename... Args>
    inline constexpr bool IsDetectedV = IsDetected<Op, Args...>::Value;
}; // namespace Prism
