/*
 * Created by v1tr10l7 on 12.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/String/String.hpp>
#include <Prism/String/StringUtils.hpp>

namespace Prism
{
    template <typename C>
    class BasicFormatterContext
    {
      public:
        using CharType = C;
        using Iterator = const CharType*;

        explicit constexpr BasicFormatterContext(
            BasicStringView<C> formatString, isize nextArgID = 0)
            : m_FormatString(formatString)
            , m_NextArgID(nextArgID)
        {
        }

        constexpr Iterator begin() const PM_NOEXCEPT
        {
            return m_FormatString.begin();
        }
        constexpr Iterator end() const PM_NOEXCEPT
        {
            return m_FormatString.end();
        }

        constexpr void AdvanceTo(Iterator it)
        {
            m_FormatString.RemovePrefix(
                StringUtils::ToNumber<usize>(it - begin()));
        }
        constexpr isize NextArgID()
        {
            if (m_NextArgID < 0)
                static_assert(
                    "cannot switch from manual to automatic argument indexing");

            isize id = m_NextArgID++;
            DoCheckArgID(id);

            return id;
        }

        constexpr void CheckArgID(isize id)
        {
            if (m_NextArgID < 0)
                static_assert(
                    "cannot switch from manual to automatic argument indexing");
            m_NextArgID = -1;
            DoCheckArgID(id);
        }

        constexpr void CheckArgID(BasicStringView<C>) {}
        constexpr void CheckDynamicSpec(isize id);

        constexpr void DoCheckArgID(isize id);

      private:
        BasicStringView<C> m_FormatString;
        isize              m_NextArgID;
    };

    enum class ArgumentType
    {
        eNone,
        eInteger,
        eUnsignedInteger,
        eLongLong,
        eUnsignedLongLong,
        eBool,
        eChar,
        eLastIntegerType = eChar,
        eCString,
        eString,
        ePointer,
        eCustom,
    };

    constexpr bool IsIntegralType(enum ArgumentType type)
    {
        return type > ArgumentType::eNone
            && type <= ArgumentType::eLastIntegerType;
    }
    constexpr bool IsArithmeticType(ArgumentType type)
    {
        return IsIntegralType(type);
    }

    template <typename C>
    class CompileFormatterContext : public BasicFormatterContext<C>
    {
        using Base = BasicFormatterContext<C>;

      public:
        explicit constexpr CompileFormatterContext(
            BasicStringView<C> formatStr, int argumentCount,
            const enum ArgumentType* types, int nextArgID = 0)
            : Base(formatStr, nextArgID)
            , m_ArgumentCount(argumentCount)
            , m_Types(types)
        {
        }

        constexpr isize ArgumentCount() const { return m_ArgumentCount; }
        constexpr enum ArgumentType ArgumentType(int id) const
        {
            return m_Types[id];
        }

        constexpr isize NextArgID()
        {
            isize id = Base::NextArgID();
            if (id >= m_ArgumentCount) static_assert("argument not found");

            return id;
        }

        constexpr void CheckArgID(isize id)
        {
            Base::CheckArgID(id);
            if (id >= m_ArgumentCount) static_assert("argument not found");
        }
        using Base::CheckArgID;

        constexpr void CheckDynamicSpec(isize argID)
        {
            IgnoreUnused(argID);
            if (argID < m_ArgumentCount && m_Types
                && !IsIntegralType(m_Types[argID]))
                static_assert("width/precision is not integer");
        }

      private:
        int                      m_ArgumentCount;
        const enum ArgumentType* m_Types;
    };

    template <typename C>
    constexpr void BasicFormatterContext<C>::DoCheckArgID(isize id)
    {
        if (IsConstantEvaluated())
        {
            using Context = CompileFormatterContext<C>;
            if (id >= static_cast<Context*>(this)->ArgumentCount())
                static_assert("argument not found");
        }
    }

    template <typename C>
    constexpr void BasicFormatterContext<C>::CheckDynamicSpec(isize argID)
    {
        if (IsConstantEvaluated())
        {
            using Context = CompileFormatterContext<C>;
            static_cast<Context*>(this)->CheckDynamicSpec(argID);
        }
    }
}; // namespace Prism
