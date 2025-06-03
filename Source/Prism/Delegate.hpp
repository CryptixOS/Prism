/*
 * Created by v1tr10l7 on 28.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Debug/Assertions.hpp>

/*
        Based on:
        - original work by Sergey Ryazanov (MIT)
        "The Impossibly Fast C++ Delegates", 18 Jul 2005
        https://www.codeproject.com/articles/11015/the-impossibly-fast-c-delegates

        - and the work of Sergey Alexandrovich Kryukov (MIT)
        "The Impossibly Fast C++ Delegates, Fixed", 11 Mar 2017
        https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed
*/

namespace Prism
{
    namespace Details
    {
        template <typename T, typename U = typename InvUnwrap<T>::Type>
        constexpr U&&
        ToLValueRef(typename RemoveReference<T>::Type& value) noexcept
        {
            return static_cast<U&&>(value);
        }

        template <typename ResultType, typename _Fn, typename... Args>
        constexpr ResultType DoInvoke(InvokeOther, _Fn&& fn, Args&&... args)
        {
            return Forward<_Fn>(fn)(Forward<Args>(args)...);
        }

        template <typename ResultType, typename MemFun, typename T,
                  typename... Args>
        constexpr ResultType DoInvoke(InvokeMemFunRef, MemFun&& fn, T&& arg,
                                      Args&&... args)
        {
            return (ToLValueRef<T>(arg).*fn)(Forward<Args>(args)...);
        }

        template <typename ResultType, typename MemFun, typename T,
                  typename... Args>
        constexpr ResultType DoInvoke(InvokeMemFunDeref, MemFun&& fn, T&& arg,
                                      Args&&... args)
        {
            return ((*Forward<T>(arg)).*fn)(Forward<Args>(args)...);
        }

        template <typename ResultType, typename MemPtr, typename T>
        constexpr ResultType DoInvoke(InvokeMemObjRef, MemPtr&& fn, T&& arg)
        {
            return ToLValueRef<T>(arg).*fn;
        }

        template <typename ResultType, typename MemPtr, typename T>
        constexpr ResultType DoInvoke(InvokeMemObjDeref, MemPtr&& fn, T&& arg)
        {
            return (*Forward<T>(arg)).*fn;
        }

        template <typename Functor, typename... Args>
        constexpr InvokeResultType<Functor, Args...>
        InvokeFunction(Functor&& fn, Args&&... args)
            PM_NOEXCEPT(IsNoThrowInvocableV<Functor, Args...>)
        {
            using Result = InvokeResult<Functor, Args...>;
            using Type   = typename Result::Type;
            using Tag    = typename Result::InvokeResultType;

            return DoInvoke<Type>(Tag{}, Forward<Functor>(fn),
                                  Forward<Args>(args)...);
        }
    }; // namespace Details

    template <typename Functor, typename... Args>
    inline constexpr InvokeResultType<Functor, Args...> Invoke(Functor&& fn,
                                                               Args&&... args)
        PM_NOEXCEPT(IsNoThrowInvocable<Functor, Args...>::Value)
    {
        return Details::InvokeFunction(Forward<Functor>(fn),
                                       Forward<Args>(args)...);
    }

    template <typename ResultType, typename Functor, typename... Args>
    constexpr EnableIfType<IsInvocableR<ResultType, Functor, Args...>::Value,
                           ResultType>
    InvokeAsType(Functor&& fn, Args&&... args) noexcept(
        IsNoThrowInvocableR<ResultType, Functor, Args...>::Value)
    {
        using Result = InvokeResult<Functor, Args...>;
        using Type   = typename Result::Type;
        using Tag    = typename Result::InvokeResultType;
        if constexpr (IsVoidV<ResultType>)
            Details::InvokeFunction<Type>(Tag{}, Forward<Functor>(fn),
                                          Forward<Args>(args)...);
        else
            return Details::InvokeFunction<Type>(Tag{}, Forward<Functor>(fn),
                                                 Forward<Args>(args)...);
    }

    template <typename Ret>
    class Delegate;

    template <typename Ret, typename... Args>
    class Delegate<Ret(Args...)>
    {
      public:
        using ObjectType   = void*;
        using FunctionType = Ret (*)(ObjectType, Args...);
        using ReturnValue  = Ret;

        struct Functor
        {
            Functor() = default;
            Functor(std::nullptr_t)
                : Object(nullptr)
                , Stub(nullptr)
                , Destroy(nullptr)
            {
            }
            Functor(ObjectType thisObject, FunctionType stub,
                    void (*destroy)(void*) = nullptr)
                : Object(thisObject)
                , Stub(stub)
                , Destroy(destroy)
            {
            }
            ~Functor()
            {
                if (Destroy) Destroy(Object);
            }

            Functor& operator=(std::nullptr_t)
            {
                if (Destroy) Destroy(Object);

                Object  = nullptr;
                Stub    = nullptr;
                Destroy = nullptr;

                return *this;
            }

            void Reset()
            {
                if (Destroy) Destroy(Object);
                Object  = nullptr;
                Stub    = nullptr;
                Destroy = nullptr;
            }

            operator bool() const { return Object != nullptr; }
            bool operator==(const Functor& other) const
            {
                return Object == other.Object && Stub == other.Stub;
            }
            bool operator!=(const Functor& other) const
            {
                return Object != other.Object || Stub != other.Stub;
            }

            ObjectType   Object    = nullptr;
            FunctionType Stub      = nullptr;
            void (*Destroy)(void*) = nullptr;
        };

        constexpr Delegate() PM_NOEXCEPT {}
        constexpr Delegate(std::nullptr_t) PM_NOEXCEPT
            : m_Functor(nullptr, nullptr, nullptr)
        {
        }
        Delegate(const Delegate& other) { m_Functor = other.m_Functor; }
        // Delegate(const Delegate& other) { CopyFrom(other); }

        Delegate(Delegate&& other) noexcept { MoveFrom(Move(other)); }
        Delegate(FunctionType f) { Bind<f>(); }

        ~Delegate() { Reset(); }

        Delegate& operator=(std::nullptr_t) PM_NOEXCEPT { Reset(); }

        Delegate& operator=(const Delegate& other)
        {
            if (this != &other)
            {
                Reset();
                CopyFrom(other);
            }
            return *this;
        }
        Delegate& operator=(const Delegate&& other)
        {
            if (this != &other)
            {
                Reset();
                MoveFrom(Move(other));
            }
            return *this;
        }
        template <typename F>
        Delegate& operator=(F&& f)
        {
            Delegate(Move(f));
            return *this;
        }
        /*
            template <typename F>
            Delegate& operator=(std::reference_wrapper<F> f) PM_NOEXCEPT
            {
                return Delegate(f).Swap(*this);
            }*/

        bool operator==(const Delegate& other) const
        {
            return m_Functor == other.m_Functor;
        }
        bool operator!=(const Delegate& other) const
        {
            return m_Functor != other.m_Functor;
        }

        inline void Swap(Delegate& other) PM_NOEXCEPT
        {
            std::swap(m_Functor.Object, other.m_Functor.Object);
            std::swap(m_Functor.Stub, other.m_Functor.Stub);
        }

        template <Ret (*Function)(Args...)>
        void Bind()
        {
            Assign(nullptr, FreeFunctionStub<Function>);
        }

        template <typename Lambda>
        void BindLambda(Lambda&& lambda)
        {
            using Decayed = std::decay_t<Lambda>;

            if constexpr (sizeof(Decayed) <= SBO_SIZE)
            {
                new (m_Storage) Decayed(Forward<Lambda>(lambda));
                Assign(m_Storage, LambdaStub<Decayed>, &DestroyLambda<Decayed>);
                return;
            }

            Decayed* heapAllocated = new Decayed(Forward<Lambda>(lambda));
            Assign(heapAllocated, LambdaStub<Decayed>,
                   &DestroyLambdaHeap<Decayed>);
        }

        template <auto Function, typename Class>
        void Bind(Class* object)
        {
            using MemberFunction      = ReturnValue (Class::*)(Args...);
            using MemberFunctionConst = ReturnValue (Class::*)(Args...) const;

            if constexpr (IsSameV<decltype(Function), MemberFunctionConst>)
                Assign(const_cast<Class*>(object),
                       ConstMemberFunctionStub<Class, Function>);
            else
            {
                static_assert(IsSameV<decltype(Function), MemberFunction>);
                Assign(reinterpret_cast<ObjectType>(object),
                       MemberFunctionStub<Class, Function>);
            }
        }

        bool IsBound() const { return m_Functor.Stub; }
        operator bool() const { return IsBound(); }

        ReturnValue Invoke(Args... args) const
        {
            assert(IsBound() && "Trying to invoke unbound delegate.");
            return /*::Prism::Invoke*/ std::invoke(
                m_Functor.Stub, m_Functor.Object, Forward<Args>(args)...);
        }
        inline ReturnValue operator()(Args... args)
        {
            return Invoke(Forward<Args>(args)...);
        }

        void Reset()
        {
            m_Functor.Reset();
            std::memset(m_Storage, 0, SBO_SIZE);
        }

      private:
        static constexpr usize SBO_SIZE  = 32;

        Functor                m_Functor = nullptr;
        alignas(void*) std::byte m_Storage[SBO_SIZE];

        void Assign(ObjectType object, FunctionType stub,
                    void (*destroy)(void*) = nullptr)
        {
            m_Functor = Functor(object, stub, destroy);
        }

        void CopyFrom(const Delegate& other)
        {
            m_Functor.Stub    = other.m_Functor.Stub;
            m_Functor.Destroy = other.m_Functor.Destroy;

            if (other.m_Functor.Object == other.m_Storage)
            {
                // Copy lambda stored in SBO
                std::memcpy(m_Storage, other.m_Storage, SBO_SIZE);
                m_Functor.Object = m_Storage;
            }
            else if (other.m_Functor.Object)
            {
                // Copy heap-allocated lambda
                m_Functor.Object = other.m_Functor.Destroy
                                     ? other.m_Functor.Destroy(
                                           other.m_Functor.Object, CopyMode{})
                                     : other.m_Functor.Object;
            }
        }

        struct CopyMode
        {
        }; // Marker type to indicate copy mode
        void MoveFrom(Delegate&& other)
        {
            m_Functor = other.m_Functor;
            if (other.m_Functor.Object == other.m_Storage)
            {
                std::memcpy(m_Storage, other.m_Storage, SBO_SIZE);
                m_Functor.Object = m_Storage;
            }
            other.m_Functor = Functor();
        }

        template <typename Class, ReturnValue (Class::*Function)(Args...)>
        static ReturnValue MemberFunctionStub(ObjectType thisObject,
                                              Args... args)
        {
            Class* p = static_cast<Class*>(thisObject);
            return (p->*Function)(Forward<Args>(args)...);
        }

        template <typename Class, ReturnValue (Class::*Function)(Args...) const>
        static ReturnValue ConstMemberFunctionStub(ObjectType objectType,
                                                   Args... args)
        {
            const Class* p = static_cast<Class*>(objectType);
            return (p->*Function)(Forward<Args>(args)...);
        }

        template <ReturnValue (*Function)(Args...)>
        static ReturnValue FreeFunctionStub(ObjectType objectType, Args... args)
        {
            return (Function)(Forward<Args>(args)...);
        }

        template <typename Lambda>
        static ReturnValue LambdaStub(ObjectType thisObject, Args... args)
        {
            Lambda* p = static_cast<Lambda*>(thisObject);
            return (p->operator())(Forward<Args>(args)...);
        }

        template <typename Lambda>
        static void* DestroyLambda(void* object, CopyMode)
        {
            return new Lambda(
                *static_cast<Lambda*>(object)); // Copy-construct a new lambda
        }

        template <typename Lambda>
        static void DestroyLambda(void* object)
        {
            reinterpret_cast<Lambda*>(object)->~Lambda();
        }

        template <typename Lambda>
        static void DestroyLambdaHeap(void* object)
        {
            delete static_cast<Lambda*>(object);
        }
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Delegate;
#endif
