/*
 * Created by v1tr10l7 on 06.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

#include <functional>
#include <utility>

namespace Prism
{
    namespace Details
    {
        template <typename T>
        inline constexpr bool IsIntegral = false;
        template <>
        inline constexpr bool IsIntegral<bool> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned char> = true;
        template <>
        inline constexpr bool IsIntegral<char8_t> = true;
        template <>
        inline constexpr bool IsIntegral<char16_t> = true;
        template <>
        inline constexpr bool IsIntegral<char32_t> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned short> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned int> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned long> = true;
        template <>
        inline constexpr bool IsIntegral<unsigned long long> = true;

        template <bool Condition, typename TrueType, typename FalseType>
        struct Conditional
        {
            using Type = TrueType;
        };
        template <typename TrueType, typename FalseType>
        struct Conditional<false, TrueType, FalseType>
        {
            using Type = FalseType;
        };
    } // namespace Details

    template <typename T, T V>
    struct IntegralConstant
    {
        static constexpr T Value = V;
        using ValueType          = T;
        using Type               = IntegralConstant;

        constexpr           operator ValueType() const { return Value; }
        constexpr ValueType operator()() const { return Value; }
    };
    using TrueType  = IntegralConstant<bool, true>;
    using FalseType = IntegralConstant<bool, false>;

    template <bool Boolean>
    using BooleanConstant = IntegralConstant<bool, Boolean>;

    template <typename T, typename U>
    struct IsSame : FalseType
    {
    };
    template <typename T>
    struct IsSame<T, T> : TrueType
    {
    };
    template <typename T, typename U>
    constexpr bool IsSameV = IsSame<T, U>::Value;

    template <typename T>
    struct RemoveCv;
    template <typename T>
    using RemoveCvType = typename RemoveCv<T>::Type;

    template <typename T, bool>
    struct EnumUnderlying
    {
    };
    template <typename T>
    struct EnumUnderlying<T, true>
    {
        // using Type = UnderlyingType<T>;
    };
    template <typename T>
    struct EnumUnderlying<T, false>
    {
        using Type = T;
    };

    template <typename T>
    struct PromoteEnum
    {
        using Type = std::conditional_t<
            std::is_enum_v<T>,
            typename EnumUnderlying<T, std::is_enum_v<T>>::Type, T>;
    };
    template <typename T>
    using PromoteEnumType = typename PromoteEnum<T>::Type;
    template <typename T, T... Ts>
    struct IntegerSequence
    {
        using Type = T;
        static constexpr usize Size() noexcept { return sizeof...(Ts); }
    };

    template <typename T, T N, T... Ts>
    auto MakeIntegerSequence()
    {
        if constexpr (N == 0) return IntegerSequence<T, Ts...>{};
        else return MakeIntegerSequence<T, N - 1, N - 1, Ts...>();
    }
    template <typename T>
    struct MakeUnsigned
    {
        using Type = void;
    };
    template <>
    struct MakeUnsigned<signed char>
    {
        using Type = unsigned char;
    };
    template <>
    struct MakeUnsigned<short>
    {
        using Type = unsigned short;
    };
    template <>
    struct MakeUnsigned<int>
    {
        using Type = unsigned int;
    };
    template <>
    struct MakeUnsigned<long>
    {
        using Type = unsigned long;
    };
    template <>
    struct MakeUnsigned<long long>
    {
        using Type = unsigned long long;
    };
    template <>
    struct MakeUnsigned<unsigned char>
    {
        using Type = unsigned char;
    };
    template <>
    struct MakeUnsigned<unsigned short>
    {
        using Type = unsigned short;
    };
    template <>
    struct MakeUnsigned<unsigned int>
    {
        using Type = unsigned int;
    };
    template <>
    struct MakeUnsigned<unsigned long>
    {
        using Type = unsigned long;
    };
    template <>
    struct MakeUnsigned<unsigned long long>
    {
        using Type = unsigned long long;
    };
    template <>
    struct MakeUnsigned<char>
    {
        using Type = unsigned char;
    };
    template <>
    struct MakeUnsigned<char8_t>
    {
        using Type = char8_t;
    };
    template <>
    struct MakeUnsigned<char16_t>
    {
        using Type = char16_t;
    };
    template <>
    struct MakeUnsigned<char32_t>
    {
        using Type = char32_t;
    };
    template <>
    struct MakeUnsigned<bool>
    {
        using Type = bool;
    };
    template <>
    struct MakeUnsigned<wchar_t>
    {
        using Type = wchar_t;
    };

    template <typename T>
    struct RemoveCvReference;

    template <usize... Indices>
    using IndexSequence = IntegerSequence<usize, Indices...>;
    template <typename T, T N>
    using MakeIntegerSequenceV = decltype(MakeIntegerSequence<T, N>());
    template <usize N>
    using MakeIndexSequenceV = MakeIntegerSequenceV<usize, N>;

    template <typename T>
    struct IsVoid;
    template <typename T>
    struct IsVoid : IsSame<void, RemoveCvType<T>>
    {
    };
    template <typename T>
    constexpr bool IsVoidV = IsVoid<T>::Value;
    template <typename T>
    struct IsNullPointer : IsSame<std::nullptr_t, RemoveCvType<T>>
    {
    };
    template <typename T>
    constexpr bool IsNullPointerV = IsNullPointer<T>::Value;
    template <typename T>
        struct IsIntegral
        : BooleanConstant
          < requires(T t, T* p,
                     void (*f)(T)) // T* parameter excludes reference types
    {
        reinterpret_cast<T>(t); // Exclude class types
        f(0);                   // Exclude enumeration types
        p + t; // Exclude everything not yet excluded but integral types
    }>{};
    template <typename T>
    constexpr bool IsIntegralV = IsIntegral<T>::Value;
    template <typename>
    struct IsFloatingPointHelper : public FalseType
    {
    };
    template <>
    struct IsFloatingPointHelper<float> : public TrueType
    {
    };
    template <>
    struct IsFloatingPointHelper<double> : public TrueType
    {
    };
    template <>
    struct IsFloatingPointHelper<long double> : public TrueType
    {
    };
#ifdef __STDCPP_FLOAT16_T__
    template <>
    struct IsFloatingPointHelper<_Float16> : public TrueType
    {
    };
#endif
#ifdef __STDCPP_FLOAT32_T__
    template <>
    struct IsFloatingPointHelper<_Float32> : public TrueType
    {
    };
#endif

#ifdef __STDCPP_FLOAT64_T__
    template <>
    struct IsFloatingPointHelper<_Float64> : public TrueType
    {
    };
#endif

#ifdef __STDCPP_FLOAT128_T__
    template <>
    struct IsFloatingPointHelper<_Float128> : public TrueType
    {
    };
#endif

#ifdef __STDCPP_BFLOAT16_T__
    template <>
    struct IsFloatingPointHelper<__gnu_cxx::__bfloat16_t> : public TrueType
    {
    };
#endif

#if !defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_FLOAT128)
    template <>
    struct IsFloatingPointHelper<__float128> : public TrueType
    {
    };
#endif

    template <typename T>
    struct IsFloatingPoint : public IsFloatingPointHelper<RemoveCvType<T>>::Type
    {
    };
    template <typename T>
    constexpr bool IsFloatingPointV = IsFloatingPoint<T>::Value;
    template <typename T>
    struct IsArray : FalseType
    {
    };
    template <typename T>
    struct IsArray<T[]> : TrueType
    {
    };

    template <typename T, usize N>
    struct IsArray<T[N]> : TrueType
    {
    };
    template <typename T>
    constexpr bool IsArrayV = IsArray<T>::Value;
    template <typename T>
    struct IsEnum : public BooleanConstant<__is_enum(T)>
    {
    };
    template <typename T>
    inline constexpr bool IsEnumV = IsEnum<T>::Value;
    template <typename T>
    struct IsUnion : public BooleanConstant<__is_union(T)>
    {
    };
    template <typename T>
    inline constexpr bool IsUnionV = IsUnion<T>::Value;
    namespace Details
    {
        template <typename T>
        IntegralConstant<bool, !IsUnion<T>::Value> Test(i32 T::*);

        template <typename>
        FalseType Test(...);
    } // namespace Details
    template <typename T>
    struct IsClass : decltype(Details::Test<T>(nullptr))
    {
    };
    template <typename T>
    struct IsReference;
    template <typename T>
    struct IsConst;
    template <typename T>
    struct IsFunction : IntegralConstant<bool, !IsConst<const T>::Value
                                                   && !IsReference<T>::Value>
    {
    };
    template <typename T>
    constexpr bool IsFunctionV = IsFunction<T>::Value;
    template <typename T>
    struct IsPointer : FalseType
    {
    };
    template <typename T>
    struct IsPointer<T*> : TrueType
    {
    };
    template <typename T>
    struct IsPointer<T* const> : TrueType
    {
    };
    template <typename T>
    struct IsPointer<T* volatile> : TrueType
    {
    };
    template <typename T>
    struct IsPointer<T* const volatile> : TrueType
    {
    };
    template <typename T>
    constexpr bool IsPointerV = IsPointer<T>::Value;
    template <typename T>
    struct IsLValueReference : FalseType
    {
    };
    template <typename T>
    struct IsLValueReference<T&> : TrueType
    {
    };
    template <typename T>
    constexpr bool IsLValueReferenceV = IsLValueReference<T>::Value;
    template <typename T>
    struct IsRValueReference : FalseType
    {
    };
    template <typename T>
    struct IsRValueReference<T&&> : TrueType
    {
    };
    template <typename T>
    constexpr bool IsRValueReferenceV = IsRValueReference<T>::Value;
    template <typename T>
    struct IsMemberPointer;
    template <typename T>
    struct IsMemberFunctionPointer;
    template <typename T>
    struct IsMemberObjectPointer
        : IntegralConstant<bool, IsMemberPointer<T>::Value
                                     && !IsMemberFunctionPointer<T>::Value>
    {
    };
    template <typename T>
    constexpr bool IsMemberObjectPointerV = IsMemberObjectPointer<T>::Value;
    template <typename T>
    struct IsMemberFunctionPointerHelper : FalseType
    {
    };
    template <typename T, typename U>
    struct IsMemberFunctionPointerHelper<T U::*> : IsFunction<T>
    {
    };
    template <typename T>
    struct IsMemberFunctionPointer
        : IsMemberFunctionPointerHelper<typename RemoveCv<T>::Type>
    {
    };
    template <typename T>
    constexpr bool IsMemberFunctionPointerV = IsMemberFunctionPointer<T>::Value;
    template <typename T>
    struct IsArithmetic;
    template <typename T>
    struct IsFundamental
        : IntegralConstant<
              bool,
              IsArithmetic<T>::Value || IsVoid<T>::Value
                  || IsSame<std::nullptr_t, typename RemoveCv<T>::Type>::Value>
    {
    };
    template <typename T>
    constexpr bool IsFundamentalV = IsFundamental<T>::Value;
    template <typename T>
    struct IsArithmetic
        : IntegralConstant<bool,
                           IsIntegral<T>::Value || IsFloatingPoint<T>::Value>
    {
    };
    template <typename T>
    constexpr bool IsArithmeticV = IsArithmetic<T>::Value;
    template <typename T>
    struct IsScalar
        : IntegralConstant<bool, IsArithmetic<T>::Value || IsEnum<T>::Value
                                     || IsPointer<T>::Value
                                     || IsMemberPointer<T>::Value
                                     || IsNullPointer<T>::Value>
    {
    };
    template <typename T>
    constexpr bool IsScalarV = IsScalar<T>::Value;
    template <typename T>
    struct IsObject
        : IntegralConstant<bool, IsScalar<T>::Value || IsArray<T>::Value
                                     || IsUnion<T>::Value || IsClass<T>::Value>
    {
    };
    template <typename T>
    constexpr bool IsObjectV = IsObject<T>::Value;
    template <typename T>
    struct IsCompound : IntegralConstant<bool, !IsFundamental<T>::Value>
    {
    };
    template <typename T>
    constexpr bool IsCompoundV = IsCompound<T>::Value;

    template <typename T>
    struct IsTriviallyCopyable
        : public BooleanConstant<__is_trivially_copyable(T)>
    {
    };
    template <typename T>
    constexpr bool IsTriviallyCopyableV = IsTriviallyCopyable<T>::Value;
    template <typename T>
    struct IsStandardLayout : public BooleanConstant<__is_standard_layout(T)>
    {
    };
    template <typename T>
    constexpr bool IsStandardLayoutV = IsStandardLayout<T>::Value;

    /*
    template <typename T>
    struct HasUniqueObjectRepresentation
        : BooleanConstant<__has_unique_object_representations(
              RemoveCvType<RemoveAllExtentsType<T>>)>
    {
    };
    template <typename T>
    constexpr bool HasUniqueObjectRepresentationV
        = HasUniqueObjectRepresentation<T>::Value;*/

    template <typename T>
    struct IsReference : FalseType
    {
    };
    template <class T>
    struct IsReference<T&> : TrueType
    {
    };
    template <typename T>
    struct IsReference<T&&> : TrueType
    {
    };
    template <typename T>
    constexpr bool IsReferenceV = IsReference<T>::Value;
    template <typename T>
    struct IsMemberPointerHelper : FalseType
    {
    };
    template <typename T, typename U>
    struct IsMemberPointerHelper<T U::*> : TrueType
    {
    };
    template <typename T>
    struct IsMemberPointer : IsMemberPointerHelper<typename RemoveCv<T>::Type>
    {
    };
    template <typename T>
    constexpr bool IsMemberPointerV = IsMemberPointer<T>::Value;
    template <typename T>
    struct IsConst : FalseType
    {
    };
    template <typename T>
    struct IsConst<const T> : TrueType
    {
    };
    template <typename T>
    constexpr bool IsConstV = IsConst<T>::Value;
    template <typename T>
    struct IsVolatile : FalseType
    {
    };
    template <typename T>
    struct IsVolatile<volatile T> : TrueType
    {
    };
    template <class T>
    constexpr bool IsVolatileV = IsVolatile<T>::Value;

    namespace Details
    {
        template <typename T, bool = IsArithmeticV<T>>
        struct IsSigned : IntegralConstant<bool, T(-1) < T(0)>
        {
        };

        template <typename T>
        struct IsSigned<T, false> : FalseType
        {
        };
    } // namespace Details

    template <typename T>
    struct IsSigned : Details::IsSigned<T>::Type
    {
    };
    template <typename T>
    constexpr bool IsSignedV = IsSigned<T>::Value;

    namespace Details
    {
        template <typename B>
        TrueType TestPointerConversion(const volatile B*);
        template <typename>
        FalseType TestPointerConversion(const volatile void*);

        template <typename B, typename D>
        auto TestIsBaseOf(int)
            -> decltype(TestPointerConversion<B>(static_cast<D*>(nullptr)));
        template <typename, typename>
        auto TestIsBaseOf(...) -> TrueType;
    } // namespace Details
    template <typename Base, typename Derived>
    struct IsBaseOf
        : IntegralConstant<
              bool,
              IsClass<Base>::Value
                  && IsClass<Derived>::Value&& decltype(Details::TestIsBaseOf<
                                                        Base, Derived>(
                      0))::Value>
    {
    };
    template <typename T>
    struct RemoveCv
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveCv<const T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveCv<volatile T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveCv<const volatile T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveConst
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveConst<const T>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveVolatile
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveVolatile<volatile T>
    {
        using Type = T;
    };
    template <typename T>
    using RemoveCvType = typename RemoveCv<T>::Type;
    template <typename T>
    using RemoveConstType = typename RemoveConst<T>::Type;
    template <typename T>
    using RemoveVolatileType = typename RemoveVolatile<T>::Type;
    template <typename T>
    struct AddCv
    {
        using Type = const volatile T;
    };
    template <typename T>
    struct AddConst
    {
        using Type = const T;
    };
    template <typename T>
    struct AddVolatile
    {
        using Type = volatile T;
    };
    template <typename T>
    using AddCvType = typename AddCv<T>::Type;
    template <typename T>
    using AddConstType = typename AddConst<T>::Type;
    template <typename T>
    using AddVolatileType = typename AddVolatile<T>::Type;
    template <typename T>
    struct RemoveReference
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveReference<T&>
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveReference<T&&>
    {
        using Type = T;
    };
    template <typename T>
    using RemoveReferenceType = typename RemoveReference<T>::Type;
    template <typename T>
    struct RemovePointer
    {
        using Type = T;
    };
    template <typename T>
    struct RemovePointer<T*>
    {
        using Type = T;
    };
    template <typename T>
    struct RemovePointer<T* const>
    {
        using Type = T;
    };
    template <typename T>
    struct RemovePointer<T* volatile>
    {
        using Type = T;
    };
    template <typename T>
    struct RemovePointer<T* const volatile>
    {
        using Type = T;
    };
    template <typename T>
    using RemovePointerType = typename RemovePointer<T>::Type;
    namespace Details
    {
        template <typename T>
        struct TypeIdentity
        {
            using Type = T;
        };

        template <typename T>
        auto TryAddPointer(int)
            -> TypeIdentity<typename RemoveReference<T>::Type*>;
        template <typename T>
        auto TryAddPointer(...) -> TypeIdentity<T>;
    } // namespace Details
    template <typename T>
    struct AddPointer : decltype(Details::TryAddPointer<T>(0))
    {
    };
    template <typename T>
    using AddPointerType = typename AddPointer<T>::Type;
    template <typename T>
    struct RemoveExtent
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveExtent<T[]>
    {
        using Type = T;
    };
    template <typename T, usize N>
    struct RemoveExtent<T[N]>
    {
        using Type = T;
    };
    template <typename T>
    using RemoveExtentType = typename RemoveExtent<T>::Type;
    template <typename T>
    struct Decay
    {
      private:
        using U = typename RemoveReference<T>::Type;

      public:
        using Type = typename Details::Conditional<
            IsArray<U>::Value,
            typename AddPointer<typename RemoveExtent<U>::Type>::Type,
            typename Details::Conditional<
                IsFunction<U>::Value, typename AddPointer<U>::Type,
                typename RemoveCv<U>::Type>::Type>::Type;
    };
    template <typename T>
    using DecayType = typename Decay<T>::Type;
    template <typename T>
    struct RemoveCvRef
    {
        using Type = RemoveCvType<RemoveReferenceType<T>>;
    };
    template <typename T>
    using RemoveCvRefType = RemoveCvRef<T>::Type;
    template <bool B, typename T = void>
    struct EnableIf
    {
    };
    template <typename T>
    struct EnableIf<true, T>
    {
        using Type = T;
    };
    template <bool B, typename T = void>
    using EnableIfType = typename EnableIf<B, T>::Type;
    template <bool B, typename T, typename F>
    struct Conditional
    {
        using Type = T;
    };
    template <typename T, typename F>
    struct Conditional<false, T, F>
    {
        using Type = F;
    };
    template <bool B, typename T, typename F>
    using ConditionalType = typename Conditional<B, T, F>::Type;
    template <typename T>
    struct UnderlyingType
    {
        using Type = __underlying_type(T);
    };
    template <typename T>
        requires(IsEnumV<T>)
    using UnderlyingTypeT = typename UnderlyingType<T>::Type;
    template <typename T>
    class ReferenceWrapper;
    namespace Details
    {
        template <typename T>
        struct IsReferenceWrapper : FalseType
        {
        };
        template <typename U>
        struct IsReferenceWrapper<ReferenceWrapper<U>> : TrueType
        {
        };

        template <typename T>
        struct InvokeImpl
        {
            template <typename F, typename... Args>
            static auto Call(F&& f, Args&&... args)
                -> decltype(std::forward<F>(f)(std::forward<Args>(args)...));
        };

        template <typename B, typename MT>
        struct InvokeImpl<MT B::*>
        {
            template <typename T, typename Td = typename Decay<T>::Type,
                      typename
                      = typename EnableIf<IsBaseOf<B, Td>::Value>::Type>
            static auto Get(T&& t) -> T&&;

            template <typename T, typename Td = typename Decay<T>::Type,
                      typename
                      = typename EnableIf<IsReferenceWrapper<Td>::Value>::Type>
            static auto Get(T&& t) -> decltype(t.Get());

            template <typename T, typename Td = typename Decay<T>::Type,
                      typename
                      = typename EnableIf<!IsBaseOf<B, Td>::Value>::Type,
                      typename
                      = typename EnableIf<!IsReferenceWrapper<Td>::Value>::Type>
            static auto Get(T&& t) -> decltype(*std::forward<T>(t));
            template <typename T, typename... Args, typename MT1,
                      typename
                      = typename EnableIf<IsFunction<MT1>::Value>::Type>
            static auto Call(MT1 B::* pmf, T&& t, Args&&... args)
                -> decltype((InvokeImpl::Get(std::forward<T>(t))
                             .*pmf)(std::forward<Args>(args)...));
            template <typename T>
            static auto Call(MT B::* pmd, T&& t)
                -> decltype(InvokeImpl::Get(std::forward<T>(t)).*pmd);
        };
        template <typename F, typename... Args,
                  typename Fd = typename Decay<F>::Type>
        auto INVOKE(F&& f, Args&&... args)
            -> decltype(InvokeImpl<Fd>::Call(std::forward<F>(f),
                                             std::forward<Args>(args)...));
        template <typename AlwaysVoid, typename, typename...>
        struct InvokeResult
        {
        };
        template <typename F, typename... Args>
        struct InvokeResult<decltype(void(Details::INVOKE(
                                std::declval<F>(), std::declval<Args>()...))),
                            F, Args...>
        {
            using Type = decltype(Details::INVOKE(std::declval<F>(),
                                                  std::declval<Args>()...));
        };
    } // namespace Details

    template <typename>
    struct ResultOf;
    template <typename F, typename... ArgTypes>
    struct ResultOf<F(ArgTypes...)>
        : Details::InvokeResult<void, F, ArgTypes...>
    {
    };
    template <typename T>
    struct RemoveAllExtents
    {
        using Type = T;
    };
    template <typename T>
    struct RemoveAllExtents<T[]>
    {
        using Type = typename RemoveAllExtents<T>::Type;
    };
    template <typename T, usize N>
    struct RemoveAllExtents<T[N]>
    {
        using Type = typename RemoveAllExtents<T>::Type;
    };
    template <typename T>
    using RemoveAllExtentsType = typename RemoveAllExtents<T>::Type;

    template <typename F, typename... ArgTypes>
    struct InvokeResult : Details::InvokeResult<void, F, ArgTypes...>
    {
    };
    template <typename T>
    using ResulfOfType = typename ResultOf<T>::Type;
    template <typename F, typename... ArgTypes>
    using InvokeResultType = typename InvokeResult<F, ArgTypes...>::Type;

    template <typename...>
    using VoidType = void;
    template <typename T>
    struct TypeIdentity
    {
        using Type = T;
    };
    template <typename T>
    using TypeIdentityType = TypeIdentity<T>::Type;

    namespace Details
    {
        template <typename T>
        constexpr T& FUN(T& t) noexcept
        {
            return t;
        }
        template <typename T>
        void FUN(T&&) = delete;
    } // namespace Details
    template <typename T>
    class ReferenceWrapper
    {
      public:
        using Type = T;

        template <typename U,
                  typename
                  = decltype(Details::FUN<T>(std::declval<U>()),
                             EnableIfType<!IsSameV<ReferenceWrapper,
                                                   RemoveCvRefType<U>>>())>
        constexpr ReferenceWrapper(U&& u) noexcept(
            noexcept(Details::FUN<T>(std::forward<U>(u))))
            : m_Pointer(std::addressof(Details::FUN<T>(std::forward<U>(u))))
        {
        }
        ReferenceWrapper(const ReferenceWrapper&) noexcept = default;
        ReferenceWrapper& operator=(const ReferenceWrapper& x) noexcept
            = default;
        constexpr    operator T&() const noexcept { return *m_Pointer; }
        constexpr T& Get() const noexcept { return *m_Pointer; }

        template <typename... ArgTypes>
        constexpr InvokeResultType<T&, ArgTypes...>
        operator()(ArgTypes&&... args) const
            noexcept(std::is_nothrow_invocable_v<T&, ArgTypes...>)
        {
            return std::invoke(Get(), std::forward<ArgTypes>(args)...);
        }

      private:
        T* m_Pointer;
    };

    template <typename T>
    ReferenceWrapper(T&) -> ReferenceWrapper<T>;
    template <typename T>
    struct UnwrapReference
    {
        using Type = T;
    };
    template <typename U>
    struct UnwrapReference<ReferenceWrapper<U>>
    {
        using Type = U&;
    };
    template <typename T>
    struct UnwrapRefDecay : UnwrapReference<DecayType<T>>
    {
    };
    template <typename T>
    using UnwrapReferenceType = UnwrapReference<T>::Type;
    template <typename T>
    using UnwrapReferenceDecayType = UnwrapRefDecay<T>::Type;

    template <typename... Args>
    struct Conjunction : TrueType
    {
    };
    template <typename B1>
    struct Conjunction<B1> : B1
    {
    };
    template <typename B1, typename... Bn>
    struct Conjunction<B1, Bn...>
        : ConditionalType<bool(B1::Value), Conjunction<Bn...>, B1>
    {
    };
    template <typename... B>
    constexpr bool ConjunctionV = Conjunction<B...>::Value;

    template <typename... Args>
    struct Disjunction : FalseType
    {
    };
    template <typename B1>
    struct Disjunction<B1> : B1
    {
    };
    template <typename B1, typename... Bn>
    struct Disjunction<B1, Bn...>
        : ConditionalType<bool(B1::Value), B1, Disjunction<Bn...>>
    {
    };
    template <typename... B>
    constexpr bool DisjunctionV = Disjunction<B...>::Value;

    template <typename B>
    struct Negation : BooleanConstant<!bool(B::Value)>
    {
    };
    template <typename B>
    constexpr bool NegationV = Negation<B>::Value;

    template <typename T>
    constexpr RemoveReferenceType<T>&& Move(T&& object) noexcept
    {
        return static_cast<RemoveReferenceType<T>&&>(object);
    }
    template <typename T>
    constexpr T&& Forward(RemoveReferenceType<T>& value) noexcept
    {
        return static_cast<T&&>(value);
    }
    template <typename T>
    constexpr T&& Forward(RemoveReferenceType<T>&& value) noexcept
    {
        static_assert(!IsLValueReferenceV<T>,
                      "Don't forward an rvalue as an lvalue");
        return static_cast<T&&>(value);
    }

}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Conditional;
using Prism::Forward;
using Prism::IsSameV;
using Prism::Move;
#endif
