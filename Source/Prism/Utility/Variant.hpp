/*
 * Created by v1tr10l7 on 09.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Bits.hpp>
#include <Prism/Core/ParameterPack.hpp>
#include <Prism/String/FormatString.hpp>

namespace Prism
{
    namespace Detail
    {
        template <typename T, usize... Is>
        constexpr auto IntegerSequenceGenerateArray(PM_UNUSED T const offset,
                                                    IntegerSequence<T, Is...>)
            -> Array<T, sizeof...(Is)>
        {
            return {{(offset + Is)...}};
        }
    } // namespace Detail

    template <typename T, T N>
    constexpr auto IotaArray(T const offset = {})
    {
        static_assert(N >= T{}, "Negative sizes not allowed in IotaArray()");
        return Detail::IntegerSequenceGenerateArray<T>(
            offset, MakeIntegerSequence<T, N>());
    }

    template <typename TEndIterator, IteratorPairWith<TEndIterator> TIterator>
    PM_NODISCARD constexpr bool AnyOf(TIterator const&    begin,
                                      TEndIterator const& end,
                                      auto const&         predicate)
    {
        return FindIf(begin, end, predicate) != end;
    }

    template <IterableContainer Container>
    PM_NODISCARD constexpr bool AnyOf(Container&& container,
                                      auto const& predicate)
    {
        return AnyOf(container.begin(), container.end(), predicate);
    }

    template <typename TEndIterator, IteratorPairWith<TEndIterator> TIterator>
    PM_NODISCARD constexpr bool AllOf(TIterator const&    begin,
                                      TEndIterator const& end,
                                      auto const&         predicate)
    {
        constexpr auto negatedPredicate = [](auto const& pred)
        { return [&](auto const& elem) { return !pred(elem); }; };
        return !(FindIf(begin, end, negatedPredicate(predicate)) != end);
    }

    template <IterableContainer Container>
    PM_NODISCARD constexpr bool AllOf(Container&& container,
                                      auto const& predicate)
    {
        return AllOf(container.begin(), container.end(), predicate);
    }

    struct Empty
    {
        constexpr bool operator==(Empty const&) const = default;
    };

    template <typename ReferenceType, typename T>
    using CopyConst = ConditionalType<IsConstV<ReferenceType>, AddConstType<T>,
                                      RemoveConstType<T>>;

    namespace Details
    {
        template <typename T, typename IndexType, IndexType InitialIndex,
                  typename... InTypes>
        struct VariantIndexOf
        {
            static_assert(false, "Invalid VariantIndex instantiated");
        };

        template <typename T, typename IndexType, IndexType InitialIndex,
                  typename InType, typename... RestOfInTypes>
        struct VariantIndexOf<T, IndexType, InitialIndex, InType,
                              RestOfInTypes...>
        {
            consteval IndexType operator()()
            {
                if constexpr (IsSameV<T, InType>) return InitialIndex;
                else
                    return VariantIndexOf<T, IndexType, InitialIndex + 1,
                                          RestOfInTypes...>{}();
            }
        };

        template <typename T, typename IndexType, IndexType InitialIndex>
        struct VariantIndexOf<T, IndexType, InitialIndex>
        {
            consteval IndexType operator()() { return InitialIndex; }
        };

        template <typename T, typename IndexType, typename... Ts>
        consteval IndexType IndexOf()
        {
            return VariantIndexOf<T, IndexType, 0, Ts...>{}();
        }

        template <typename IndexType, IndexType InitialIndex, typename... Args>
        struct Variant;

        template <typename IndexType, IndexType InitialIndex, typename F,
                  typename... Args>
        struct Variant<IndexType, InitialIndex, F, Args...>
        {
          public:
            static constexpr auto CURRENT_INDEX
                = VariantIndexOf<F, IndexType, InitialIndex, F, Args...>{}();
            PM_ALWAYS_INLINE static void Delete(IndexType id, void* data)
            {
                if (id == CURRENT_INDEX) BitCast<F*>(data)->~F();
                else
                    Variant<IndexType, InitialIndex + 1, Args...>::Delete(id,
                                                                          data);
            }

            PM_ALWAYS_INLINE static void Move(IndexType oldID, void* oldData,
                                              void* newData)
            {
                if (oldID == CURRENT_INDEX)
                    new (newData) F(Prism::Move(*BitCast<F*>(oldData)));
                else
                    Variant<IndexType, InitialIndex + 1, Args...>::Move(
                        oldID, oldData, newData);
            }

            PM_ALWAYS_INLINE static void
            Copy(IndexType oldID, void const* oldData, void* newData)
            {
                if (oldID == CURRENT_INDEX)
                    new (newData) F(*BitCast<F const*>(oldData));
                else
                    Variant<IndexType, InitialIndex + 1, Args...>::Copy(
                        oldID, oldData, newData);
            }
        };

        template <typename IndexType, IndexType InitialIndex>
        struct Variant<IndexType, InitialIndex>
        {
            PM_ALWAYS_INLINE static void Delete(IndexType, void*) {}
            PM_ALWAYS_INLINE static void Move(IndexType, void*, void*) {}
            PM_ALWAYS_INLINE static void Copy(IndexType, void const*, void*) {}
        };

        template <typename IndexType, typename... Args>
        struct VisitImpl
        {
            template <typename RT, typename T, usize I, typename Fn>
            static constexpr bool HasExplicitlyNamedOverload()
            {
                return requires {
                    (DeclVal<Fn>().*(&Fn::operator()))(DeclVal<T>());
                };
            }

            template <typename ReturnType, typename T, typename Visitor,
                      auto... Is>
            static constexpr bool
            ShouldInvokeConstOverload(IndexSequence<Is...>)
            {
                return ((HasExplicitlyNamedOverload<
                            ReturnType, T, Is,
                            typename Visitor::Types::template Type<Is>>())
                        || ...);
            }

            template <typename Self, typename Visitor,
                      IndexType CurrentIndex = 0>
            PM_ALWAYS_INLINE static constexpr decltype(auto)
            Visit(Self& self, IndexType id, void const* data, Visitor&& visitor)
                requires(CurrentIndex < sizeof...(Args))
            {
                using T = typename ParameterPack<Args...>::template Type<
                    CurrentIndex>;

                if (id == CurrentIndex)
                {
                    using ReturnType = decltype(visitor(*BitCast<T*>(data)));
                    if constexpr (ShouldInvokeConstOverload<ReturnType, T,
                                                            Visitor>(
                                      MakeIndexSequence<
                                          Visitor::Types::Size>()))
                        return visitor(*BitCast<AddConstType<T>*>(data));

                    return visitor(*BitCast<CopyConst<Self, T>*>(data));
                }

                if constexpr ((CurrentIndex + 1) < sizeof...(Args))
                    return Visit<Self, Visitor, CurrentIndex + 1>(
                        self, id, data, Forward<Visitor>(visitor));
                else PrismNotReached();
            }
        };

        struct VariantNoClearTag
        {
            explicit VariantNoClearTag() = default;
        };
        struct VariantConstructTag
        {
            explicit VariantConstructTag() = default;
        };

        template <typename T, typename Base>
        struct VariantConstructors
        {
            PM_ALWAYS_INLINE VariantConstructors(T&& t)
                requires(requires {
                    T(Prism::Move(t));
                    typename Base;
                })
            {
                InternalCast().ClearWithoutDestruction();
                InternalCast().Set(Prism::Move(t), VariantNoClearTag{});
            }

            PM_ALWAYS_INLINE VariantConstructors(T const& t)
                requires(requires {
                    T(t);
                    typename Base;
                })
            {
                InternalCast().ClearWithoutDestruction();
                InternalCast().Set(t, VariantNoClearTag{});
            }

            PM_ALWAYS_INLINE VariantConstructors() = default;

          private:
            PM_NODISCARD PM_ALWAYS_INLINE Base& InternalCast()
            {
                return *static_cast<Base*>(this);
            }
        };

        struct ParameterPackTag
        {
        };

        template <typename... Ts>
        struct ParameterPack : ParameterPackTag
        {
        };

        template <typename T>
        struct Blank
        {
        };

        template <typename A, typename P>
        inline constexpr bool IsTypeInPack = false;

        template <typename T, typename... Ts>
        inline constexpr bool IsTypeInPack<T, ParameterPack<Ts...>>
            = (IsSameV<T, Ts> || ...);

        template <typename T, typename... Qs>
        using BlankIfDuplicate
            = ConditionalType<(IsTypeInPack<T, Qs> || ...), Blank<T>, T>;

        template <usize I, typename P, typename Seq, typename... Qs>
        struct InheritFromUniqueEntries;

        template <usize I, typename... Ts, usize... Js, typename... Qs>
        struct InheritFromUniqueEntries<I, ParameterPack<Ts...>,
                                        IndexSequence<Js...>, Qs...>
            : public BlankIfDuplicate<
                  Ts, ConditionalType<Js <= I, ParameterPack<>, Qs>...>...
        {

            using BlankIfDuplicate<Ts,
                                   ConditionalType<Js <= I, ParameterPack<>,
                                                   Qs>...>::BlankIfDuplicate...;
        };

        template <typename...>
        struct InheritFromPacks;

        template <usize... Is, typename... Ps>
        struct InheritFromPacks<IndexSequence<Is...>, Ps...>
            : public InheritFromUniqueEntries<Is, Ps, IndexSequence<Is...>,
                                              Ps...>...
        {

            using InheritFromUniqueEntries<Is, Ps, IndexSequence<Is...>,
                                           Ps...>::InheritFromUniqueEntries...;
        };

        template <typename... Ps>
        using MergeAndDeduplicatePacks
            = InheritFromPacks<MakeIndexSequence<sizeof...(Ps)>,
                               ConditionalType<IsBaseOfV<ParameterPackTag, Ps>,
                                               Ps, ParameterPack<Ps>>...>;
    }; // namespace Details
    template <typename T>
    concept NotLValueReference = !IsLValueReferenceV<T>;

    template <NotLValueReference... Ts>
    struct Variant : public Details::MergeAndDeduplicatePacks<
                         Details::VariantConstructors<Ts, Variant<Ts...>>...>
    {
      public:
        using IndexType = ConditionalType<(sizeof...(Ts) < 255), u8,
                                          usize>; // Note: size+1 reserved for
                                                  // internal value checks
      private:
        static constexpr IndexType INVALID_INDEX = sizeof...(Ts);

        template <typename T>
        static constexpr IndexType IndexOf()
        {
            return Details::IndexOf<T, IndexType, Ts...>();
        }

      public:
        template <typename T>
        static constexpr bool CanContain()
        {
            return IndexOf<T>() != INVALID_INDEX;
        }

        template <typename... NewTs>
        Variant(Variant<NewTs...>&& old)
            requires((CanContain<NewTs>() && ...))
            : Variant(Prism::Move(old).template DownCast<Ts...>())
        {
        }

        template <typename... NewTs>
        Variant(Variant<NewTs...> const& old)
            requires((CanContain<NewTs>() && ...))
            : Variant(old.template DownCast<Ts...>())
        {
        }

        template <NotLValueReference... NewTs>
        friend struct Variant;

        Variant()
            requires(!CanContain<Empty>())
        = delete;
        Variant()
            requires(CanContain<Empty>())
            : Variant(Empty())
        {
        }

        Variant(Variant const&)
            requires(!(IsCopyConstructibleV<Ts> && ...))
        = delete;
        Variant(Variant const&) = default;

        Variant(Variant&&)
            requires(!(IsMoveConstructibleV<Ts> && ...))
        = delete;
        Variant(Variant&&) = default;

        ~Variant()
            requires(!(IsDestructibleV<Ts> && ...))
        = delete;
        ~Variant() = default;

        Variant& operator=(Variant const&)
            requires(!(IsCopyConstructibleV<Ts> && ...)
                     || !(IsDestructibleV<Ts> && ...))
        = delete;
        Variant& operator=(Variant const&) = default;

        Variant& operator=(Variant&&)
            requires(!(IsMoveConstructibleV<Ts> && ...)
                     || !(IsDestructibleV<Ts> && ...))
        = delete;
        Variant&         operator=(Variant&&) = default;

        PM_ALWAYS_INLINE Variant(Variant const& old)
            requires(!(IsTriviallyCopyConstructibleV<Ts> && ...))
            : Details::MergeAndDeduplicatePacks<
                  Details::VariantConstructors<Ts, Variant<Ts...>>...>()
            , m_Data{}
            , m_Index(old.m_Index)
        {
            Helper::Copy(old.m_Index, old.m_Data, m_Data);
        }

        PM_ALWAYS_INLINE Variant(Variant&& old)
            requires(!(IsTriviallyMoveConstructibleV<Ts> && ...))
            : Details::MergeAndDeduplicatePacks<
                  Details::VariantConstructors<Ts, Variant<Ts...>>...>()
            , m_Index(old.m_Index)
        {
            Helper::Move(old.m_Index, old.m_Data, m_Data);
        }

        PM_ALWAYS_INLINE ~Variant()
            requires(!(IsTriviallyDestructibleV<Ts> && ...))
        {
            Helper::Delete(m_Index, m_Data);
        }

        PM_ALWAYS_INLINE Variant& operator=(Variant const& other)
            requires(!(IsTriviallyCopyConstructibleV<Ts> && ...)
                     || !(IsTriviallyDestructibleV<Ts> && ...))
        {
            if (this != &other)
            {
                if constexpr (!(IsTriviallyDestructibleV<Ts> && ...))
                {
                    Helper::Delete(m_Index, m_Data);
                }
                m_Index = other.m_Index;
                Helper::Copy(other.m_Index, other.m_Data, m_Data);
            }
            return *this;
        }

        PM_ALWAYS_INLINE Variant& operator=(Variant&& other)
            requires(!(IsTriviallyMoveConstructibleV<Ts> && ...)
                     || !(IsTriviallyDestructibleV<Ts> && ...))
        {
            if (this != &other)
            {
                if constexpr (!(IsTriviallyDestructibleV<Ts> && ...))
                {
                    Helper::Delete(m_Index, m_Data);
                }
                m_Index = other.m_Index;
                Helper::Move(other.m_Index, other.m_Data, m_Data);
            }
            return *this;
        }

        using Details::MergeAndDeduplicatePacks<Details::VariantConstructors<
            Ts, Variant<Ts...>>...>::MergeAndDeduplicatePacks;

        template <typename T, typename StrippedT = RemoveCvRefType<T>>
        void Set(T&& t)
            requires(CanContain<StrippedT>()
                     && requires { StrippedT(Forward<T>(t)); })
        {
            constexpr auto NEW_INDEX = IndexOf<StrippedT>();
            Helper::Delete(m_Index, m_Data);
            new (m_Data) StrippedT(Forward<T>(t));
            m_Index = NEW_INDEX;
        }

        template <typename T, typename StrippedT = RemoveCvRefType<T>>
        void Set(T&& t, Details::VariantNoClearTag)
            requires(CanContain<StrippedT>()
                     && requires { StrippedT(Forward<T>(t)); })
        {
            constexpr auto NEW_INDEX = IndexOf<StrippedT>();
            new (m_Data) StrippedT(Forward<T>(t));
            m_Index = NEW_INDEX;
        }

        template <typename T>
        T* GetPointer()
            requires(CanContain<T>())
        {
            if (IndexOf<T>() == m_Index) return BitCast<T*>(&m_Data);
            return nullptr;
        }

        template <typename T>
        T& Get()
            requires(CanContain<T>())
        {
            assert(Has<T>());
            return *BitCast<T*>(&m_Data);
        }

        template <typename T>
        T const* GetPointer() const
            requires(CanContain<T>())
        {
            if (IndexOf<T>() == m_Index) return BitCast<T const*>(&m_Data);
            return nullptr;
        }

        template <typename T>
        T const& Get() const
            requires(CanContain<T>())
        {
            assert(Has<T>());
            return *BitCast<T const*>(&m_Data);
        }

        template <typename T>
        PM_NODISCARD bool Has() const
            requires(CanContain<T>())
        {
            return IndexOf<T>() == m_Index;
        }

        bool operator==(Variant const& other) const
        {
            return this->Visit(
                [&]<typename T>(T const& self)
                {
                    if (auto const* p = other.GetPointer<T>())
                        return static_cast<T const&>(self)
                            == static_cast<T const&>(*p);
                    return false;
                });
        }

        template <typename... Fs>
        PM_ALWAYS_INLINE decltype(auto) Visit(Fs&&... functions)
        {
            Visitor<Fs...> visitor{Forward<Fs>(functions)...};
            return VisitHelper::Visit(*this, m_Index, m_Data,
                                      Prism::Move(visitor));
        }

        template <typename... Fs>
        PM_ALWAYS_INLINE decltype(auto) Visit(Fs&&... functions) const
        {
            Visitor<Fs...> visitor{Forward<Fs>(functions)...};
            return VisitHelper::Visit(*this, m_Index, m_Data,
                                      Prism::Move(visitor));
        }

        template <typename... NewTs>
        decltype(auto) DownCast() &&
        {
            if constexpr (sizeof...(NewTs) == 1
                          && (IsSpecializationOfV<NewTs, Variant> && ...))
            {
                return Prism::Move(*this).template DownCastVariant<NewTs...>();
            }
            else {
                Variant<NewTs...> instance{Variant<NewTs...>::INVALID_INDEX,
                                           Details::VariantConstructTag{}};
                Visit(
                    [&](auto& value)
                    {
                        if constexpr (Variant<NewTs...>::template CanContain<
                                          RemoveCvRefType<decltype(value)>>())
                            instance.Set(Prism::Move(value),
                                         Details::VariantNoClearTag{});
                    });
                assert(instance.m_Index != instance.INVALID_INDEX);
                return instance;
            }
        }

        template <typename... NewTs>
        decltype(auto) DownCast() const&
        {
            if constexpr (sizeof...(NewTs) == 1
                          && (IsSpecializationOfV<NewTs, Variant> && ...))
            {
                return (*this).DownCastVariant(TypeWrapper<NewTs...>{});
            }
            else {
                Variant<NewTs...> instance{Variant<NewTs...>::INVALID_INDEX,
                                           Details::VariantConstructTag{}};
                Visit(
                    [&](auto const& value)
                    {
                        if constexpr (Variant<NewTs...>::template CanContain<
                                          RemoveCvRefType<decltype(value)>>())
                            instance.Set(value, Details::VariantNoClearTag{});
                    });
                assert(instance.m_Index != instance.INVALID_INDEX);
                return instance;
            }
        }

        auto Index() const { return m_Index; }

      private:
        template <typename... NewTs>
        Variant<NewTs...> DownCastVariant(TypeWrapper<Variant<NewTs...>>) &&
        {
            return Prism::Move(*this).template DownCast<NewTs...>();
        }

        template <typename... NewTs>
        Variant<NewTs...> DownCastVariant(TypeWrapper<Variant<NewTs...>>) const&
        {
            return (*this).template DownCast<NewTs...>();
        }

        static constexpr auto DATA_SIZE
            = Detail::IntegerSequenceGenerateArray<usize>(
                  0, IntegerSequence<usize, sizeof(Ts)...>())
                  .Max();
        static constexpr auto DATA_ALIGNMENT
            = Detail::IntegerSequenceGenerateArray<usize>(
                  0, IntegerSequence<usize, alignof(Ts)...>())
                  .Max();
        using Helper      = Details::Variant<IndexType, 0, Ts...>;
        using VisitHelper = Details::VisitImpl<IndexType, Ts...>;

        template <typename T_, typename U_>
        friend struct Details::VariantConstructors;

        explicit Variant(IndexType index, Details::VariantConstructTag)
            : Details::MergeAndDeduplicatePacks<
                  Details::VariantConstructors<Ts, Variant<Ts...>>...>()
            , m_Index(index)
        {
        }

        PM_ALWAYS_INLINE void ClearWithoutDestruction()
        {
            Memory::Fill(m_Data, 0, DATA_SIZE);
            m_Index = INVALID_INDEX;
        }

        template <typename... Fs>
        struct Visitor : Fs...
        {
            using Types = ParameterPack<Fs...>;

            Visitor(Fs&&... args)
                : Fs(Forward<Fs>(args))...
            {
            }

            using Fs::operator()...;
        };

        alignas(DATA_ALIGNMENT) u8 m_Data[DATA_SIZE];
        IndexType m_Index;
    };

    template <typename... Ts>
    struct ParameterPack<Variant<Ts...>> : ParameterPack<Ts...>
    {
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Variant;
#endif
