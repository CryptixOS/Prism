/*
 * Created by v1tr10l7 on 12.07.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/String/StringView.hpp>

/**
 * Building blocks of the Prism formatting library: the customization point
 * (Formatter<T>), type-erased arguments, the output buffer and the parse and
 * format contexts.
 *
 * Nothing here allocates. Output always goes through a FormatBuffer, which
 * is a fixed window that the concrete buffer flushes somewhere (a String, an
 * output iterator, a log sink, a counter, ...) whenever it fills up.
 */
namespace Prism
{
    /**
     * Customization point. Specialize it for your own types:
     *
     *     template <>
     *     struct Prism::Formatter<Foo> : Prism::Formatter<Prism::StringView>
     *     {
     *         auto Format(const Foo& foo, Prism::FormatContext& ctx) const
     *         {
     *             return Prism::FormatTo(ctx.Out(), "Foo({})", foo.Value);
     *         }
     *     };
     *
     * - `Parse(FormatParseContext&)` is optional. It must be constexpr, since
     *   it also runs when the format string is checked at compile time. It
     *   returns the position of the closing '}' of the replacement field.
     *   Without it, the type accepts no format specifiers.
     * - `Format(const T&, FormatContext&)` writes the value and returns
     *   ctx.Out() (or nothing).
     *
     * The primary template is deliberately unusable: a type is formattable
     * only through a specialization or a FormatAs(T) overload found by ADL.
     */
    template <typename T, typename Char = char>
    struct Formatter
    {
        Formatter() = delete;
    };

    enum class FormatArgType : u8
    {
        eNone,
        eInt,  // any signed integer, stored as i64
        eUInt, // any unsigned integer, stored as u64
        eBool,
        eChar,
        eLastInteger = eChar,
        eCString,
        eString,
        ePointer,
        eCustom,
    };

    inline constexpr bool IsIntegralArgType(FormatArgType type)
    {
        return type > FormatArgType::eNone
            && type <= FormatArgType::eLastInteger;
    }

    class FormatParseContext;
    class FormatContext;

    namespace Details
    {
        /// Never defined as constexpr on purpose: reaching it while a format
        /// string is checked at compile time makes the call ill-formed, and
        /// the compiler points at the offending format string.
        inline void FormatStringError(const char* message) { (void)message; }

        /// Reports a malformed format string. At compile time this is a hard
        /// error; at run time (Runtime() format strings) the faulty field is
        /// skipped, just like the patched fmt did.
        constexpr void ReportFormatError(const char* message)
        {
            if consteval
            {
                FormatStringError(message);
            }
            (void)message;
        }

        inline constexpr bool NameEquals(const char* name, const char* str,
                                  usize length)
        {
            for (usize i = 0; i < length; ++i)
                if (name[i] != str[i]) return false;
            return name[length] == '\0';
        }
    } // namespace Details

    /**
     * A type-erased formatting argument.
     */
    struct FormatArg
    {
        using CustomFormatFn
            = void (*)(const void*, FormatParseContext&, FormatContext&);

        struct StringValue
        {
            const char* Data;
            usize       Size;
        };
        struct CustomValue
        {
            const void*    Value;
            CustomFormatFn Format;
        };

        FormatArgType Type = FormatArgType::eNone;
        union
        {
            i64         Int;
            u64         UInt;
            bool        Bool;
            char        Char;
            const char* CString;
            StringValue String;
            const void* Pointer;
            CustomValue Custom;
        };

        constexpr FormatArg()
            : UInt(0)
        {
        }

        constexpr explicit operator bool() const
        {
            return Type != FormatArgType::eNone;
        }
    };

    struct FormatNamedArgInfo
    {
        const char* Name;
        isize       ID;
    };

    /**
     * A view of a list of type-erased arguments, see MakeFormatArgs().
     */
    class FormatArgs
    {
      public:
        constexpr FormatArgs() = default;
        constexpr FormatArgs(const FormatArg* args, usize count,
                             const FormatNamedArgInfo* named = nullptr,
                             usize                     namedCount = 0)
            : m_Args(args)
            , m_Count(count)
            , m_Named(named)
            , m_NamedCount(namedCount)
        {
        }

        constexpr usize     Size() const { return m_Count; }

        constexpr FormatArg Get(isize id) const
        {
            if (id < 0 || static_cast<usize>(id) >= m_Count) return {};
            return m_Args[id];
        }
        constexpr isize GetID(StringView name) const
        {
            for (usize i = 0; i < m_NamedCount; ++i)
                if (Details::NameEquals(m_Named[i].Name, name.Raw(),
                                        name.Size()))
                    return m_Named[i].ID;
            return -1;
        }
        constexpr FormatArg Get(StringView name) const
        {
            return Get(GetID(name));
        }

      private:
        const FormatArg*          m_Args       = nullptr;
        usize                     m_Count      = 0;
        const FormatNamedArgInfo* m_Named      = nullptr;
        usize                     m_NamedCount = 0;
    };

    /**
     * Output window of the formatter. Concrete buffers decide what happens
     * when it is full (flush it to a String, an iterator, a counter...).
     */
    class FormatBuffer
    {
      public:
        FormatBuffer(const FormatBuffer&)            = delete;
        FormatBuffer& operator=(const FormatBuffer&) = delete;

        void          PushBack(char c)
        {
            if (m_Size == m_Capacity) Grow();
            m_Data[m_Size++] = c;
        }
        void Append(const char* data, usize length)
        {
            while (length > 0)
            {
                if (m_Size == m_Capacity) Grow();
                usize chunk = m_Capacity - m_Size;
                if (chunk > length) chunk = length;

                __builtin_memcpy(m_Data + m_Size, data, chunk);
                m_Size += chunk;
                data += chunk;
                length -= chunk;
            }
        }
        void Append(StringView str) { Append(str.Raw(), str.Size()); }
        void Append(const char* begin, const char* end)
        {
            Append(begin, static_cast<usize>(end - begin));
        }
        /// Appends `count` copies of a (possibly multi-byte) fill sequence.
        void Fill(usize count, const char* fill, usize fillSize)
        {
            if (fillSize == 1)
            {
                while (count > 0)
                {
                    if (m_Size == m_Capacity) Grow();
                    usize chunk = m_Capacity - m_Size;
                    if (chunk > count) chunk = count;

                    __builtin_memset(m_Data + m_Size, *fill, chunk);
                    m_Size += chunk;
                    count -= chunk;
                }
                return;
            }
            for (; count > 0; --count) Append(fill, fillSize);
        }

        /// Characters currently held in the window (not flushed yet).
        usize       Size() const { return m_Size; }
        const char* Data() const { return m_Data; }

      protected:
        FormatBuffer(char* data, usize capacity)
            : m_Data(data)
            , m_Capacity(capacity)
        {
        }
        ~FormatBuffer() = default;

        /// Called when the window is full; must leave room for at least one
        /// more character.
        virtual void Grow() = 0;

        char*        m_Data;
        usize        m_Size = 0;
        usize        m_Capacity;
    };

    /**
     * FormatBuffer with inline storage that hands every full window to
     * `Sink(const char* data, usize size)`. Call Flush() when done.
     */
    template <typename Sink, usize Capacity = 256>
    class SinkFormatBuffer final : public FormatBuffer
    {
      public:
        explicit SinkFormatBuffer(Sink sink)
            : FormatBuffer(m_Storage, Capacity)
            , m_Sink(Move(sink))
        {
        }

        void Flush()
        {
            if (m_Size > 0) m_Sink(m_Data, m_Size);
            m_Size = 0;
        }
        Sink& GetSink() { return m_Sink; }

      protected:
        void Grow() override { Flush(); }

      private:
        char m_Storage[Capacity];
        Sink m_Sink;
    };

    /**
     * Output iterator writing into a FormatBuffer. This is what
     * FormatContext::Out() returns; FormatTo() recognizes it and writes
     * straight into the underlying buffer.
     */
    class FormatAppender
    {
      public:
        using ValueType      = char;
        using DifferenceType = isize;

        constexpr explicit FormatAppender(FormatBuffer& buffer)
            : m_Buffer(&buffer)
        {
        }

        FormatAppender& operator=(char c)
        {
            m_Buffer->PushBack(c);
            return *this;
        }
        constexpr FormatAppender& operator*() { return *this; }
        constexpr FormatAppender& operator++() { return *this; }
        constexpr FormatAppender  operator++(int) { return *this; }

        constexpr FormatBuffer&   Buffer() const { return *m_Buffer; }

      private:
        FormatBuffer* m_Buffer;
    };

    /**
     * Parsing state handed to Formatter<T>::Parse(): the remaining part of
     * the format string plus argument-id bookkeeping. When the format string
     * is checked at compile time it also knows the argument types.
     */
    class FormatParseContext
    {
      public:
        using Iterator = const char*;

        constexpr explicit FormatParseContext(StringView     format,
                                              isize          argCount = -1,
                                              const FormatArgType* types
                                              = nullptr)
            : m_Begin(format.Raw())
            , m_End(format.Raw() + format.Size())
            , m_ArgCount(argCount)
            , m_Types(types)
        {
        }

        constexpr Iterator begin() const PM_NOEXCEPT { return m_Begin; }
        constexpr Iterator end() const PM_NOEXCEPT { return m_End; }
        constexpr void     AdvanceTo(Iterator it) { m_Begin = it; }

        /// Next automatic argument id (`{}`).
        constexpr isize    NextArgID()
        {
            if (m_NextArgID < 0)
            {
                Details::ReportFormatError(
                    "cannot switch from manual to automatic argument "
                    "indexing");
                return 0;
            }
            isize id = m_NextArgID++;
            CheckIndex(id);
            return id;
        }
        /// Manual argument id (`{0}`).
        constexpr void CheckArgID(isize id)
        {
            if (m_NextArgID > 0)
            {
                Details::ReportFormatError(
                    "cannot switch from automatic to manual argument "
                    "indexing");
                return;
            }
            m_NextArgID = -1;
            CheckIndex(id);
        }
        /// Named argument id (`{name}`).
        constexpr void CheckArgID(StringView) {}

        /// The argument `id` is used as a dynamic width or precision.
        constexpr void CheckDynamicSpec(isize id)
        {
            if (!m_Types || id < 0 || id >= m_ArgCount) return;
            auto type = m_Types[id];
            if (type != FormatArgType::eInt && type != FormatArgType::eUInt)
                Details::ReportFormatError("width/precision is not integer");
        }

      private:
        Iterator             m_Begin;
        Iterator             m_End;
        isize                m_NextArgID = 0;
        isize                m_ArgCount;
        const FormatArgType* m_Types;

        constexpr void       CheckIndex(isize id)
        {
            if (m_ArgCount >= 0 && id >= m_ArgCount)
                Details::ReportFormatError("argument not found");
        }
    };

    /**
     * Formatting state handed to Formatter<T>::Format(): the output and the
     * arguments (needed for dynamic width/precision).
     */
    class FormatContext
    {
      public:
        using Iterator = FormatAppender;

        FormatContext(FormatBuffer& buffer, FormatArgs args)
            : m_Buffer(buffer)
            , m_Args(args)
        {
        }

        Iterator          Out() { return Iterator(m_Buffer); }
        void              AdvanceTo(Iterator) {}

        FormatBuffer&     Buffer() { return m_Buffer; }
        const FormatArgs& Args() const { return m_Args; }

        FormatArg         Arg(isize id) const { return m_Args.Get(id); }
        FormatArg         Arg(StringView name) const { return m_Args.Get(name); }
        isize ArgID(StringView name) const { return m_Args.GetID(name); }

      private:
        FormatBuffer& m_Buffer;
        FormatArgs    m_Args;
    };

    /// Runtime named argument, see Arg().
    template <typename T>
    struct NamedArg
    {
        const char* Name;
        const T&    Value;
    };

    /// fmt::arg: `Format("{answer}", Arg("answer", 42))`.
    template <typename T>
    constexpr NamedArg<T> Arg(const char* name, const T& value)
    {
        return {name, value};
    }

    /// String usable as a template argument (for "name"_a literals).
    template <usize N>
    struct FixedFormatString
    {
        char Data[N]{};

        consteval FixedFormatString(const char (&str)[N])
        {
            for (usize i = 0; i < N; ++i) Data[i] = str[i];
        }
    };

    /// Compile-time named argument, see FormatLiterals::operator""_a.
    template <FixedFormatString Name, typename T>
    struct StaticNamedArg
    {
        const T& Value;
    };

    template <FixedFormatString Name>
    struct StaticNamedArgBuilder
    {
        template <typename T>
        constexpr StaticNamedArg<Name, T> operator=(const T& value) const
        {
            return {value};
        }
    };

    namespace FormatLiterals
    {
        /// `Format("{answer}", "answer"_a = 42)`. Unlike Arg(), names given
        /// this way are verified at compile time.
        template <FixedFormatString Name>
        constexpr auto operator""_a()
        {
            return StaticNamedArgBuilder<Name>{};
        }
    } // namespace FormatLiterals
}; // namespace Prism
