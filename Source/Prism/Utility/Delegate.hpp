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

PRISM_EXPORT namespace Prism
{
    namespace Details
    {
        template <typename T, typename U = typename InvUnwrap<T>::Type>
        constexpr U&&
        ToLValueRef(typename RemoveReference<T>::Type& value) noexcept
        {
            return static_cast<U&&>(value);
        }

        template <typename ResultType, typename Fn, typename... Args>
        constexpr ResultType DoInvoke(InvokeOther, Fn&& fn, Args&&... args)
        {
            if constexpr (IsVoidV<ResultType>)
                Forward<Fn>(fn)(Forward<Args>(args)...);
            else return Forward<Fn>(fn)(Forward<Args>(args)...);
        }

        template <typename ResultType, typename MemFun, typename T,
                  typename... Args>
        constexpr ResultType DoInvoke(InvokeMemFunRef, MemFun&& fn, T&& arg,
                                      Args&&... args)
        {
            if constexpr (IsVoidV<ResultType>)
                (ToLValueRef<T>(arg).*fn)(Forward<Args>(args)...);
            else return (ToLValueRef<T>(arg).*fn)(Forward<Args>(args)...);
        }

        template <typename ResultType, typename MemFun, typename T,
                  typename... Args>
        constexpr ResultType DoInvoke(InvokeMemFunDeref, MemFun&& fn, T&& arg,
                                      Args&&... args)
        {
            if constexpr (IsVoidV<ResultType>)
                ((*Forward<T>(arg)).*fn)(Forward<Args>(args)...);
            else return ((*Forward<T>(arg)).*fn)(Forward<Args>(args)...);
        }

        template <typename ResultType, typename MemPtr, typename T>
        constexpr ResultType DoInvoke(InvokeMemObjRef, MemPtr&& fn, T&& arg)
        {
            if constexpr (IsVoidV<ResultType>) ToLValueRef<T>(arg).*fn;
            else return ToLValueRef<T>(arg).*fn;
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
            using Tag    = decltype(GetInvokeTag(DeclVal<Functor>(),
                                                 DeclVal<Args>()...));

            if constexpr (IsVoidV<InvokeResultType<Functor, Args...>>)
                DoInvoke<Type>(Tag{}, Forward<Functor>(fn),
                               Forward<Args>(args)...);
            else
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
        if constexpr (IsVoidV<ResultType>)
            ::Prism::Invoke(Forward<Functor>(fn), Forward<Args>(args)...);
        else
            return ::Prism::Invoke(Forward<Functor>(fn),
                                   Forward<Args>(args)...);
    }

    template <typename Ret>
    class Delegate;

    /**
     * @brief Type-erased callable with small-buffer optimization.
     *
     * A delegate can hold:
     *  - a free function, bound at compile time (Bind<&Fn>()) or at runtime
     *    (a function pointer),
     *  - a member function plus a non-owning object pointer
     *    (Bind<&Class::Method>(object)),
     *  - an owned callable (lambda/functor). It lives inline when it fits in
     *    SBO_SIZE, is suitably aligned and is nothrow-movable, and on the heap
     *    otherwise.
     *
     * Equality: free functions and member bindings compare by
     * (object, function). Owned callables only compare equal to themselves,
     * because captured state can't be compared in general.
     */
    template <typename Ret, typename... Args>
    class Delegate<Ret(Args...)>
    {
      public:
        using ObjectType   = void*;
        using FunctionType = Ret (*)(ObjectType, Args...);
        using ReturnValue  = Ret;

        static constexpr usize SBO_SIZE      = 32;
        static constexpr usize SBO_ALIGNMENT = alignof(void*) * 2;

        constexpr Delegate() PM_NOEXCEPT = default;
        constexpr Delegate(NullType) PM_NOEXCEPT {}

        Delegate(const Delegate& other) { CopyFrom(other); }
        Delegate(Delegate&& other) PM_NOEXCEPT { MoveFrom(other); }

        template <typename F>
            requires(IsFunctionPointerV<DecayType<F>>
                     && IsInvocableR<Ret, DecayType<F>, Args...>::Value)
        Delegate(F f)
        {
            BindFunctionPointer(f);
        }
        template <typename F>
            requires(!IsSameV<RemoveCvRefType<F>, Delegate>
                     && !IsSameV<RemoveCvRefType<F>, NullType>
                     && !IsFunctionV<RemoveReferenceType<F>>
                     && !IsFunctionPointerV<DecayType<F>>
                     && IsInvocableR<Ret, DecayType<F>&, Args...>::Value)
        Delegate(F&& f)
        {
            BindLambda(Forward<F>(f));
        }

        ~Delegate() { Reset(); }

        Delegate& operator=(NullType) PM_NOEXCEPT
        {
            Reset();
            return *this;
        }
        Delegate& operator=(const Delegate& other)
        {
            if (this != &other)
            {
                Delegate copy(other);
                Reset();
                MoveFrom(copy);
            }
            return *this;
        }
        Delegate& operator=(Delegate&& other) PM_NOEXCEPT
        {
            if (this != &other)
            {
                Reset();
                MoveFrom(other);
            }
            return *this;
        }
        template <typename F>
            requires(!IsSameV<RemoveCvRefType<F>, Delegate>
                     && !IsSameV<RemoveCvRefType<F>, NullType>
                     && IsInvocableR<Ret, DecayType<F>&, Args...>::Value)
        Delegate& operator=(F&& f)
        {
            Delegate tmp(Forward<F>(f));
            Reset();
            MoveFrom(tmp);
            return *this;
        }

        bool operator==(const Delegate& other) const
        {
            return m_Stub == other.m_Stub && m_Object == other.m_Object;
        }
        bool operator!=(const Delegate& other) const
        {
            return !(*this == other);
        }

        void Swap(Delegate& other) PM_NOEXCEPT
        {
            if (this == &other) return;
            Delegate tmp(Move(other));
            other = Move(*this);
            *this = Move(tmp);
        }

        template <Ret (*Function)(Args...)>
        void Bind()
        {
            Reset();
            m_Stub = &FreeFunctionStub<Function>;
        }

        /// Binds a member function. `object` is not owned and must outlive
        /// the delegate. Works with const objects for const methods and
        /// with methods inherited from a base class.
        template <auto Method, typename Class>
        void Bind(Class* object)
        {
            static_assert(IsMemberFunctionPointerV<decltype(Method)>,
                          "Bind<Method>(object) expects a member function");
            // Convert to the class that declares Method now, while the static
            // type is known, so the stub casts void* back to the exact type
            // that was stored.
            using Owner  = typename MemberClassOf<decltype(Method)>::Type;
            using Target = ConditionalType<IsConstV<Class>, const Owner, Owner>;
            Target* target = object;

            Reset();
            m_Object = const_cast<Owner*>(target);
            m_Stub   = &MethodStub<Method, Target>;
        }

        template <typename Lambda>
        void BindLambda(Lambda&& lambda)
        {
            using Decayed = DecayType<Lambda>;
            Reset();

            if constexpr (FitsInline<Decayed>())
            {
                m_Object = new (m_Storage) Decayed(Forward<Lambda>(lambda));
                m_Manager = &InlineManager<Decayed>;
            }
            else
            {
                m_Object  = new Decayed(Forward<Lambda>(lambda));
                m_Manager = &HeapManager<Decayed>;
            }
            m_Stub = &LambdaStub<Decayed>;
        }

        template <Ret (*Function)(Args...)>
        static Delegate FromFunction()
        {
            Delegate d;
            d.Bind<Function>();
            return d;
        }
        template <auto Method, typename Class>
        static Delegate FromMethod(Class* instance)
        {
            Delegate d;
            d.Bind<Method>(instance);
            return d;
        }

        PM_NODISCARD bool IsBound() const PM_NOEXCEPT
        {
            return m_Stub != nullptr;
        }
        operator bool() const PM_NOEXCEPT { return IsBound(); }
        /// @brief Whether the callable lives in the inline buffer.
        PM_NODISCARD bool IsInline() const PM_NOEXCEPT
        {
            return m_Object == static_cast<const void*>(m_Storage);
        }

        ReturnValue Invoke(Args... args) const
        {
            assert(IsBound() && "Trying to invoke unbound delegate.");
            return m_Stub(m_Object, Forward<Args>(args)...);
        }
        ReturnValue operator()(Args... args) const
        {
            return Invoke(Forward<Args>(args)...);
        }

        void Reset() PM_NOEXCEPT
        {
            if (m_Manager) m_Manager(Operation::eDestroy, this, nullptr);
            m_Object  = nullptr;
            m_Stub    = nullptr;
            m_Manager = nullptr;
        }

      private:
        template <typename M>
        struct MemberClassOf;
        template <typename M, typename C>
        struct MemberClassOf<M C::*>
        {
            using Type = C;
        };

        enum class Operation
        {
            eDestroy,
            eCopy,
            eMove,
        };
        /// eDestroy: destroys self's callable.
        /// eCopy/eMove: builds a copy of / steals other's callable into self.
        /// The manager itself sets self->m_Object.
        using ManagerType = void (*)(Operation, Delegate* self,
                                     Delegate* other);

        ObjectType  m_Object  = nullptr;
        FunctionType m_Stub   = nullptr;
        ManagerType m_Manager = nullptr;
        alignas(SBO_ALIGNMENT) u8 m_Storage[SBO_SIZE];

        template <typename F>
        static constexpr bool FitsInline()
        {
            return sizeof(F) <= SBO_SIZE && alignof(F) <= SBO_ALIGNMENT
                && IsNoThrowMoveConstructibleV<F>;
        }

        template <typename F>
        void BindFunctionPointer(F f)
        {
            using FnPtr = DecayType<F>;
            Reset();
            if (!f) return;
            m_Object = reinterpret_cast<ObjectType>(f);
            m_Stub   = &FunctionPointerStub<FnPtr>;
        }

        void CopyFrom(const Delegate& other)
        {
            m_Stub = other.m_Stub;
            if (other.m_Manager)
            {
                m_Manager = other.m_Manager;
                m_Manager(Operation::eCopy, this,
                          const_cast<Delegate*>(&other));
            }
            else m_Object = other.m_Object;
        }
        void MoveFrom(Delegate& other) PM_NOEXCEPT
        {
            m_Stub = other.m_Stub;
            if (other.m_Manager)
            {
                m_Manager = other.m_Manager;
                m_Manager(Operation::eMove, this, &other);
            }
            else m_Object = other.m_Object;

            other.m_Object  = nullptr;
            other.m_Stub    = nullptr;
            other.m_Manager = nullptr;
        }

        template <typename F>
        static void InlineManager(Operation op, Delegate* self,
                                  Delegate* other)
        {
            switch (op)
            {
                case Operation::eDestroy:
                    static_cast<F*>(self->m_Object)->~F();
                    break;
                case Operation::eCopy:
                    if constexpr (IsCopyConstructibleV<F>)
                        self->m_Object = new (self->m_Storage)
                            F(*static_cast<const F*>(other->m_Object));
                    else
                        assert(false && "Copying a move-only callable");
                    break;
                case Operation::eMove:
                {
                    F* source      = static_cast<F*>(other->m_Object);
                    self->m_Object = new (self->m_Storage) F(Move(*source));
                    source->~F();
                    break;
                }
            }
        }
        template <typename F>
        static void HeapManager(Operation op, Delegate* self, Delegate* other)
        {
            switch (op)
            {
                case Operation::eDestroy:
                    delete static_cast<F*>(self->m_Object);
                    break;
                case Operation::eCopy:
                    if constexpr (IsCopyConstructibleV<F>)
                        self->m_Object
                            = new F(*static_cast<const F*>(other->m_Object));
                    else
                        assert(false && "Copying a move-only callable");
                    break;
                case Operation::eMove:
                    self->m_Object = other->m_Object;
                    break;
            }
        }

        template <typename Fn>
        static ReturnValue FunctionPointerStub(ObjectType object, Args... args)
        {
            auto fn = reinterpret_cast<Fn>(object);
            return fn(Forward<Args>(args)...);
        }
        template <auto Method, typename Class>
        static ReturnValue MethodStub(ObjectType object, Args... args)
        {
            Class* instance = static_cast<Class*>(object);
            return (instance->*Method)(Forward<Args>(args)...);
        }
        template <Ret (*Function)(Args...)>
        static ReturnValue FreeFunctionStub(ObjectType, Args... args)
        {
            return Function(Forward<Args>(args)...);
        }
        template <typename Lambda>
        static ReturnValue LambdaStub(ObjectType object, Args... args)
        {
            return (*static_cast<Lambda*>(object))(Forward<Args>(args)...);
        }
    };
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
PRISM_EXPORT using Prism::Delegate;
#endif
