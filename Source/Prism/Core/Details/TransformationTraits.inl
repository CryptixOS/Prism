/*
 * Created by v1tr10l7 on 01.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/TypeProperties.inl>

#include <functional>

namespace Prism
{
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

#if PrismHasBuiltin(__remove_all_extents)
    template <typename T>
    struct RemoveAllExtents
    {
        using Type = __remove_all_extents(T);
    };
#else
    template <typename T>
    struct RemoveAllExtents
    {
        using Type = T;
    };

    template <typename T, usize Size>
    struct RemoveAllExtents<T[Size]>
    {
        using Type = typename RemoveAllExtents<T>::Type;
    };

    template <typename T>
    struct RemoveAllExtents<T[]>
    {
        using Type = typename RemoveAllExtents<T>::Type;
    };
#endif

    template <typename T>
    struct Decay
    {
      private:
        using U = typename RemoveReference<T>::Type;

      public:
        using Type = typename Conditional<
            IsArray<U>::Value,
            typename AddPointer<typename RemoveExtent<U>::Type>::Type,
            typename Conditional<IsFunction<U>::Value,
                                 typename AddPointer<U>::Type,
                                 typename RemoveCv<U>::Type>::Type>::Type;
    };

    namespace Details
    {
        template <typename...>
        using VoidType = void;

        template <typename T, typename U>
        using ConditionalResultType
            = decltype(false ? DeclVal<T>() : DeclVal<U>());

        template <typename, typename, typename = void>
        struct DecayConditionalResult
        {
        };
        template <typename T, typename U, typename = void>
        struct CommonType2 : DecayConditionalResult<const T&, const U&>
        {
        };

        template <typename T, typename U>
        struct CommonType2<T, U, VoidType<ConditionalResultType<T, U>>>
            : DecayConditionalResult<T, U>
        {
        };
    }; // namespace Details

    // TODO(v1tr10l7): Specializations for TupleLike, BasicConstIterator
    template <typename... Args>
    struct CommonType
    {
    };
    template <typename T>
    struct CommonType<T> : CommonType<T, T>
    {
    };
    template <typename T, typename U>
    struct CommonType<T, U>
        : Conditional<IsSame<T, typename Decay<T>::Type>::Value
                          && IsSame<U, typename Decay<U>::Type>::Value,
                      Details::CommonType2<T, U>,
                      CommonType<typename Decay<T>::Type,
                                 typename Decay<U>::Type>>::Type
    {
    };
    namespace Details
    {
        template <typename AlwaysVoid, typename T, typename U,
                  typename... Other>
        struct CommonTypeMulti
        {
        };
        template <typename T, typename U, typename... Other>
        struct CommonTypeMulti<VoidType<typename CommonType<T, U>::Type>, T, U,
                               Other...>
            : CommonType<typename CommonType<T, U>::Type, Other...>
        {
        };
    }; // namespace Details

    template <typename T, typename U, typename... Other>
    struct CommonType<T, U, Other...>
        : Details::CommonTypeMulti<void, T, U, Other...>
    {
    };

    // TODO(v1tr10l7): CommonReference
    template <typename T>
    struct UnderlyingType
    {
        using Type = __underlying_type(T);
    };

    // TODO(v1tr10l7): VoidType, TypeIdentity

    namespace Details
    {
        template <typename T>
        constexpr T& CopyConstruct(T& t) PM_NOEXCEPT
        {
            return t;
        }
        template <typename T>
        void CopyConstruct(T&&) = delete;
        template <typename>
        class ReferenceWrapper;
        template <typename T>
        struct IsReferenceWrapper : FalseType
        {
        };
        template <typename U>
        struct IsReferenceWrapper<ReferenceWrapper<U>> : TrueType
        {
        };
    }; // namespace Details

    template <typename Fn, typename... ArgTypes>
    struct InvokeResult;
    template <typename Fn, typename... ArgTypes>
    struct IsNoThrowInvocable;

    template <typename T>
    class ReferenceWrapper
    {
      public:
        using Type = T;

        template <typename U,
                  typename
                  = decltype(Details::CopyConstruct<T>(DeclVal<U>()),
                             EnableIfType<!IsSameV<ReferenceWrapper,
                                                   RemoveCvRefType<U>>>())>
        constexpr ReferenceWrapper(U&& u) noexcept(
            noexcept(Details::CopyConstruct<T>(Forward<U>(u))))
            : m_Pointer(
                  std::addressof(Details::CopyConstruct<T>(Forward<U>(u))))
        {
        }
        ReferenceWrapper(const ReferenceWrapper&) PM_NOEXCEPT = default;
        ReferenceWrapper& operator=(const ReferenceWrapper& x) PM_NOEXCEPT
            = default;
        constexpr    operator T&() const PM_NOEXCEPT { return *m_Pointer; }
        constexpr T& Get() const PM_NOEXCEPT { return *m_Pointer; }

        template <typename... ArgTypes>
        constexpr typename InvokeResult<T&, ArgTypes...>::Type
        operator()(ArgTypes&&... args) const
            PM_NOEXCEPT(IsNoThrowInvocable<T&, ArgTypes...>::Value)
        {
            return Invoke(Get(), Forward<ArgTypes>(args)...);
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
    template <typename T>
    struct UnwrapReference<ReferenceWrapper<T>>
    {
        using Type = T&;
    };
    template <typename T>
    struct UnwrapReferenceDecay : UnwrapReference<typename Decay<T>::Type>
    {
    };

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
    using DecayType = typename Decay<T>::Type;
    template <typename... Types>
    using CommonTypeType = typename CommonType<Types...>::Type;

    template <typename T>
        requires(IsEnumV<T>)
    using UnderlyingTypeType = typename UnderlyingType<T>::Type;
    template <typename T>
    using UnwrapReferenceType = UnwrapReference<T>::Type;
    template <typename T>
    using UnwrapReferenceDecayType = UnwrapReferenceDecay<T>::Type;

    template <typename T>
    inline constexpr auto ToUnderlying(T value) PM_NOEXCEPT
    {
        return static_cast<UnderlyingTypeType<T>>(value);
    }
}; // namespace Prism
