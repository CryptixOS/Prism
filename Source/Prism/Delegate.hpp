/*
 * Created by v1tr10l7 on 28.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/Types.hpp>
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
            {
            }
            Functor(ObjectType thisObject, FunctionType stub)
                : Object(thisObject)
                , Stub(stub)
            {
            }

            Functor& operator=(std::nullptr_t)
            {
                Object = nullptr;
                Stub   = nullptr;

                return *this;
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

            ObjectType   Object = nullptr;
            FunctionType Stub   = nullptr;
        };

        constexpr Delegate() PM_NOEXCEPT {}
        constexpr Delegate(std::nullptr_t) PM_NOEXCEPT
            : m_Functor(nullptr, nullptr)
        {
        }
        ~Delegate() { Reset(); }

        Delegate(const Delegate& other) { m_Functor = other.m_Functor; }
        /*
            Delegate(Delegate&& other) PM_NOEXCEPT
            {
                m_Functor       = other.m_Functor;
                other.m_Functor = nullptr;
            }
            template <typename F>
            Delegate(F&& f)
                : Delegate(Delegate(std::move(f)))
            {
            }
    */
        Delegate& operator=(const Delegate& other)
        {
            m_Functor = other.m_Functor;
            return *this;
        }
        /*
            Delegate& operator=(Delegate&& other)
            {
                m_Functor       = other.m_Functor;
                other.m_Functor = nullptr;

                return *this;
            }
            Delegate& operator=(std::nullptr_t) PM_NOEXCEPT { m_Functor =
           nullptr; } template <typename F> Delegate& operator=(F&& f)
            {
                Delegate(std::forward<F&&>(f)).Swap(*this);
                return *this;
            }
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
        void BindLambda(const Lambda& lambda)
        {
            Assign((ObjectType)(&lambda), LambdaStub<Lambda>);
        }

        template <auto Function, typename Class>
        void Bind(Class* object)
        {
            using MemberFunction      = ReturnValue      (Class::*)(Args...);
            using MemberFunctionConst = ReturnValue (Class::*)(Args...) const;

            if constexpr (std::is_same_v<decltype(Function),
                                         MemberFunctionConst>)
                Assign(const_cast<Class*>(object),
                       ConstMemberFunctionStub<Class, Function>);
            else
            {
                static_assert(
                    std::is_same_v<decltype(Function), MemberFunction>);
                Assign(reinterpret_cast<ObjectType>(object),
                       MemberFunctionStub<Class, Function>);
            }
        }

        bool IsBound() const { return m_Functor.Stub; }
        operator bool() const { return IsBound(); }

        ReturnValue Invoke(Args... args) const
        {
            assert(!IsBound() && "Trying to invoke unbound delegate.");
            return std::invoke(m_Functor.Stub, m_Functor.Object,
                               std::forward<Args>(args)...);
        }
        inline ReturnValue operator()(Args... args)
        {
            return Invoke(std::forward<Args>(args)...);
        }

        void Reset() { m_Functor = Functor(); }

      private:
        Functor m_Functor = nullptr;

        void    Assign(ObjectType object, FunctionType stub)
        {
            m_Functor.Object = object;
            m_Functor.Stub   = stub;
        }
        template <typename Class, ReturnValue (Class::*Function)(Args...)>
        static ReturnValue MemberFunctionStub(ObjectType thisObject,
                                              Args... args)
        {
            Class* p = static_cast<Class*>(thisObject);
            return (p->*Function)(std::forward<Args>(args)...);
        }

        template <typename Class, ReturnValue (Class::*Function)(Args...) const>
        static ReturnValue ConstMemberFunctionStub(ObjectType objectType,
                                                   Args... args)
        {
            const Class* p = static_cast<Class*>(objectType);
            return (p->*Function)(std::forward<Args>(args)...);
        }

        template <ReturnValue (*Function)(Args...)>
        static ReturnValue FreeFunctionStub(ObjectType objectType, Args... args)
        {
            return (Function)(std::forward<Args>(args)...);
        }

        template <typename Lambda>
        static ReturnValue LambdaStub(ObjectType thisObject, Args... args)
        {
            Lambda* p = static_cast<Lambda*>(thisObject);
            return (p->operator())(std::forward<Args>(args)...);
        }
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Delegate;
#endif
